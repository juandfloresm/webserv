#include "Connection.hpp"

const char Connection::CONFIG_SEP = '=';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Connection::Connection(std::string config)
{
	processConfig(config);
	initServer();
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Connection::~Connection()
{
	SocketDataEvents e = this->_serverEvents;
	SocketDataEventsIterator it = e.begin();
	for( it = e.begin(); it != e.end(); it++)
		close(it->first);
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

std::ostream & operator<<( std::ostream & o, Connection const & i )
{
	(void) i;
	o << "Connection class" << std::endl;
	return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void Connection::processConfig(std::string config)
{
	readFile(CONFIG, Connection::processConfigLine);
	readFile(config, Connection::processConfigLine);
}

void Connection::processConfigLine( Connection & i, std::string line )
{
	std::string key, value;
	std::istringstream f(line);
	getline(f, key, Connection::CONFIG_SEP);
	getline(f, value, Connection::CONFIG_SEP);
	i._config[key] = value;
}

void Connection::handleSigint( int sgn )
{
	(void) sgn;
	kill(0, SIGKILL);
}

void Connection::initServer( void )
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ft_error("[Error] Singal problem");
		return ;
	}
	if (signal(SIGINT, handleSigint) == SIG_ERR) {
		ft_error("[Error] Singal problem");
		return ;
	}
	this->_epollfd = epoll_create(MAX_EVENTS);
	if (this->_epollfd < 0)
		ft_error("[Error] creating epoll");
	initServers();
	eventLoop();
}

void Connection::initServers( void )
{
	int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
	if (serverSocket != -1)
	{
		int _enable = 1;
		if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &_enable, sizeof(_enable)) == 0) {
			if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &_enable, sizeof(_enable)) == 0) 
			{				
				if (connect(serverSocket, this->_config) == -1)
					ft_error("[Error] binding client socket");
			}
		}
	}
	else
		ft_error("[Error] opening server socket");
}

int Connection::connect( int serverSocket, Config config )
{
	int port = geti(config, "port");
	int connections = geti(config, "connections");
	sockaddr_in serverAddress;
	memset(&serverAddress, '\0', sizeof(sockaddr_in));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) != -1)
	{
		if (listen(serverSocket, connections) == 0)
		{
			std::cout << "[Info] server is accepting HTTP connections on port: " << port << std::endl;
			struct epoll_event pollEvent;
			pollEvent.events = EPOLLIN;
			pollEvent.data.fd = serverSocket;
			SocketData data;
			data.pollEvent = pollEvent;
			data.config = config;
			this->_serverEvents[serverSocket] = data;
			if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, serverSocket, &pollEvent) == -1)
				ft_error("[Error] adding new listeding socket to epoll");
		}
		return 0;
	}
	return -1;
}

void Connection::eventLoop( void )
{
	int newEvents, sockConnectionFD;
	while(true)
	{
		newEvents = epoll_wait(this->_epollfd, this->_events, MAX_EVENTS, -1);
		if (newEvents == -1)
			ft_error("[Error] with epoll_wait");
		int serverSocket = 0;
		for (int i = 0; i < newEvents; ++i)
		{
			serverSocket = this->_events[i].data.fd;
			if (this->_serverEvents.find(serverSocket) != this->_serverEvents.end())
			{
				sockaddr_in clientAddress;
				socklen_t clientAddressSize;
				memset(&clientAddress, '\0', sizeof(sockaddr_in));
				clientAddressSize = sizeof(clientAddress);
				sockConnectionFD = accept(this->_events[i].data.fd, (struct sockaddr *)&clientAddress, &clientAddressSize);
				this->_clientEvents[sockConnectionFD] = this->_serverEvents[serverSocket];
				if (sockConnectionFD == -1)
					ft_error("[Error] accepting new connection");
				this->_serverEvents[serverSocket].pollEvent.events = EPOLLIN | EPOLLET;
				this->_serverEvents[serverSocket].pollEvent.data.fd = sockConnectionFD;
				if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, sockConnectionFD, &this->_serverEvents[serverSocket].pollEvent) == -1)
					ft_error("[Error] adding new event to epoll");
			}
			else
			{
				int newSocketFD = this->_events[i].data.fd;
				if (newSocketFD != -1)
					processClientRequest(newSocketFD);
				else
				{
					epoll_ctl(this->_epollfd, EPOLL_CTL_DEL, newSocketFD, NULL);
					close(newSocketFD);
					ft_error("[Error] accepting client connection");
				}
			}
		}
	}
}

void Connection::processClientRequest( int clientSocketFD )
{
	SocketDataEvents e = this->_clientEvents;
	SocketDataEventsIterator it = e.find(clientSocketFD);
	if (it != e.end())
	{
		Config config = it->second.config;
		Request req(clientSocketFD);
		if (req.getMethod() == UNKNOWN)
			Response res(NOT_IMPLEMENTED, clientSocketFD, *this, config, req);
		else
			Response res(OK, clientSocketFD, *this, config, req);
		e.erase(it);
	}
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

void Connection::ft_error(const std::string msg) const
{
	perror(msg.c_str());
	std::cerr << "[Error] on the event loop" << std::endl;
}

std::string Connection::gets(Config m, std::string key) const
{
	if (m.find(key) == m.end()) {
		return "";
	} else {
		return m[key];
	}
}

int Connection::geti(Config m, std::string key) const
{
	if (m.find(key) == m.end()) {
		return 0;
	} else {
		return atoi(m[key].c_str());
	}
}

float Connection::getf(Config m, std::string key) const
{
	if (m.find(key) == m.end()) {
		return 0;
	} else {
		return atof(m[key].c_str());
	}
}

void Connection::readFile( std::string file, void (*f)( Connection & i, std::string line ) )
{
	std::ifstream ifs(file.c_str());
	if (ifs.good())
	{
		int reading = 0;
		std::string	line;
		while (getline(ifs, line))
		{
			reading = 1;
			f(*this, line);
		}
		if (!reading)
			std::cerr << "Error: The file is empty" << std::endl;
		ifs.close();
	}
	else
		std::cerr << "Error: could not open file" << std::endl;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

