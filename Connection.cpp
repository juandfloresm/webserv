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
	close(this->_serverSocket);
}

/*
** --------------------------------- OVERLOAD ---------------------------------
*/

std::ostream & operator<<( std::ostream & o, Connection const & i )
{
	o << "Server listening on port = " << i.getPort();
	return o;
}

/*
** --------------------------------- METHODS ----------------------------------
*/

void Connection::processConfig(std::string config)
{
	std::cout << std::endl << "....... CONFIG .............." << std::endl << std::endl;
	readFile(CONFIG, Connection::processConfigLine);
	std::cout << std::endl << "....... CONFIG DELTA ........" << std::endl << std::endl;
	readFile(config, Connection::processConfigLine);
	std::cout << std::endl << "............................." << std::endl << std::endl;
}

void Connection::processConfigLine( Connection & i, std::string line )
{
	std::string key, value;
	std::istringstream f(line);
	getline(f, key, Connection::CONFIG_SEP);
	getline(f, value, Connection::CONFIG_SEP);
	std::cout << key << ": " << value << std::endl;
	i._config[key] = value;
}

void Connection::initServer( void )
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ft_error("[Error] Singal problem");
		return ;
	}
	
	this->_serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);

	if (this->_serverSocket != -1)
	{
		memset(&this->_serverAddress, '\0', sizeof(sockaddr_in));
		memset(&this->_clientAddress, '\0', sizeof(sockaddr_in));
		this->_serverAddress.sin_family = AF_INET;
		this->_serverAddress.sin_port = htons(geti("port"));
		this->_serverAddress.sin_addr.s_addr = INADDR_ANY;
		this->_clientAddressSize = sizeof(this->_clientAddress);

		int _enable = 1;
		if(setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEADDR, &_enable, sizeof(_enable)) == 0) {
			if(setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEPORT, &_enable, sizeof(_enable)) == 0) 
			{				
				if (connect() == -1)
				{
					/* Handle binding error */
					ft_error("[Error] binding client socket");
				}
			}
		}
	}
	else
	{
		/* Handle socket error */
		ft_error("[Error] opening server socket");
	}
}

int Connection::connect()
{
	if (bind(this->_serverSocket, (struct sockaddr *) &this->_serverAddress, sizeof(this->_serverAddress)) != -1)
	{
		if (listen(this->_serverSocket, geti("connections")) == 0)
		{
			std::cout << "[Info] server is accepting HTTP connections on port: " << geti("port") << std::endl;
			eventLoop();
		}
		return 0;
	}
	return -1;
}


void Connection::simpleServer( void )
{
	while (true)
	{
		this->_clientSocket = accept(this->_serverSocket, (struct sockaddr *) &this->_clientAddress, &this->_clientAddressSize);
		if (this->_clientSocket != -1)
			processClientRequest(this->_clientSocket);
		else
		{
			/* Handle accept error */
			std::cerr << "[Error] accepting client connection" << std::endl;
		}
	}
}

void Connection::eventLoop( void )
{
	preparePolling();
	int newEvents, sockConnectionFD;
	while(true)
	{
		newEvents = epoll_wait(this->_epollfd, this->_events, MAX_EVENTS, -1);
		if (newEvents == -1)
			ft_error("[Error] with epoll_wait");
		for (int i = 0; i < newEvents; ++i)
		{
			if (this->_events[i].data.fd == this->_serverSocket)
			{
				sockConnectionFD = accept(this->_serverSocket, (struct sockaddr *)&this->_clientAddress, &this->_clientAddressSize);
				if (sockConnectionFD == -1)
				{
					ft_error("[Error] accepting new connection");
				}
				this->_pollEvent.events = EPOLLIN | EPOLLET;
				this->_pollEvent.data.fd = sockConnectionFD;
				if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, sockConnectionFD, &this->_pollEvent) == -1)
				{
					ft_error("[Error] adding new event to epoll");
				}
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

void Connection::preparePolling( void )
{
	this->_epollfd = epoll_create(MAX_EVENTS);
	if (this->_epollfd < 0)
	{
		ft_error("[Error] creating epoll");
	}
	this->_pollEvent.events = EPOLLIN;
	this->_pollEvent.data.fd = this->_serverSocket;

	if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, this->_serverSocket, &this->_pollEvent) == -1)
	{
		ft_error("[Error] adding new listeding socket to epoll");
	}
}

void Connection::processClientRequest( int clientSocketFD )
{
	this->_clientSocket = clientSocketFD;
	std::string line = getMessageLine();
	std::string method, resource, major, minor;
	std::istringstream f(line);
	getline(f, method, ' ');
	getline(f, resource, ' ');
	getline(f, major, '/');
	getline(f, major, '.');
	getline(f, minor, '.');

	Method m = UNKNOWN;
	if (method.find("GET") != std::string::npos)
		m = GET;
	else if (method.find("POST") != std::string::npos)
		m = POST;
	else if (method.find("POST") != std::string::npos)
		m = POST;
	else if (method.find("DELETE") != std::string::npos)
		m = DELETE;
	else
	{
		Request req(m, resource, "", atoi(major.c_str()), atoi(minor.c_str()));
		Response res(NOT_IMPLEMENTED, geti("major_version"), geti("minor_version"), *this, req);
		return;
	}

	Request req(m, resource, "", atoi(major.c_str()), atoi(minor.c_str()));
	Response res(OK, geti("major_version"), geti("minor_version"), *this, req);
}

std::string Connection::getMessageLine( void )
{
	std::string line = "";
	char c, p = ' ';
	while (recv(this->_clientSocket, this->_buffer, 1, 0) > 0)
	{
		c = this->_buffer[0];
		if (c == LF || c == CR)
		{
			if (p == CR)
				break;
			else
			{
				p = c;
				continue;
			}
		}
		line.push_back(c);
	}
	return line;
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

void Connection::ft_error(const std::string msg) const
{
	perror(msg.c_str());
	std::cerr << "[Error] on the event loop" << std::endl;
}

std::string Connection::gets(std::string key) const
{
	Config m = this->_config;
	if (m.find(key) == m.end()) {
		return "";
	} else {
		return m[key];
	}
}

int Connection::geti(std::string key) const
{
	Config m = this->_config;
	if (m.find(key) == m.end()) {
		return 0;
	} else {
		return atoi(m[key].c_str());
	}
}

float Connection::getf(std::string key) const
{
	Config m = this->_config;
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

int Connection::getPort( void ) const
{
	return this->_port;
}

int Connection::getServerSocket() const
{
	return this->_serverSocket;
}

int Connection::getClientSocket() const
{
	return this->_clientSocket;
}
