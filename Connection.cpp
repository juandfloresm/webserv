#include "Connection.hpp"

const char Connection::CONFIG_SEP = '=';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Connection::Connection( Configuration & cfg ) : _cfg(cfg)
{
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

void Connection::initServer( void )
{
	if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		ft_error("Singal problem");
		return ;
	}
	if (signal(SIGINT, handleSigint) == SIG_ERR) {
		ft_error("Singal problem");
		return ;
	}
	this->_epollfd = epoll_create(MAX_EVENTS);
	if (this->_epollfd < 0)
		ft_error("Creating epoll");
	initServers();
	eventLoop();
}

void Connection::initServers( void )
{
	ServerList list = this->_cfg.getServerList();
	ServerListIterator it = list.begin();
	for (; it < list.end(); it++)
	{
		int serverSocket = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, IPPROTO_TCP);
		if (serverSocket != -1)
		{
			int _enable = 1;
			if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &_enable, sizeof(_enable)) == 0) {
				if(setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &_enable, sizeof(_enable)) == 0) 
				{				
					if (connect(serverSocket, it->getPort(), 10) == -1) // TODO
						ft_error("Binding client socket");
				}
			}
		}
		else
			ft_error("Opening server socket");
	}
}

int Connection::connect( int serverSocket, int port, int connections )
{
	sockaddr_in serverAddress;
	memset(&serverAddress, '\0', sizeof(sockaddr_in));
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(port);
	serverAddress.sin_addr.s_addr = INADDR_ANY;
	if (bind(serverSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) != -1)
	{
		if (listen(serverSocket, connections) == 0)
		{
			std::cout << "[Info] server is accepting HTTP connections on port: '" << port << "'" << std::endl;
			struct epoll_event pollEvent;
			pollEvent.events = EPOLLIN;
			pollEvent.data.fd = serverSocket;
			SocketData data;
			data.pollEvent = pollEvent;
			data.port = port;
			this->_serverEvents[serverSocket] = data;
			if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, serverSocket, &pollEvent) == -1)
				ft_error("Adding new listeding socket to epoll");
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
			ft_error("With epoll_wait");
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
					ft_error("Accepting new connection");
				this->_serverEvents[serverSocket].pollEvent.events = EPOLLIN | EPOLLET;
				this->_serverEvents[serverSocket].pollEvent.data.fd = sockConnectionFD;
				if (epoll_ctl(this->_epollfd, EPOLL_CTL_ADD, sockConnectionFD, &this->_serverEvents[serverSocket].pollEvent) == -1)
					ft_error("Adding new event to epoll");
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
					ft_error("Accepting client connection");
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
		Request req(clientSocketFD, _cfg);
		if (req.getMethod() == UNKNOWN)
			Response res(NOT_IMPLEMENTED, clientSocketFD, _cfg, it->second.port, req);
		else
			Response res(OK, clientSocketFD, _cfg, it->second.port, req);
		e.erase(it);
	}
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

void Connection::handleSigint( int sgn )
{
	if (sgn == SIGINT)
		kill(0, SIGKILL);
}

void Connection::ft_error(const std::string msg) const
{
	if (errno != 0)
		perror(" ");
	std::cerr << "[Error] " << msg << std::endl;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

Configuration & Connection::getConfiguration( void ) const
{
	return this->_cfg;
}
