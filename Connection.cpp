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
	std::cout << std::endl << "......................." << std::endl << std::endl;
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
					/* Handle accept error */
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
	this->_index = 0;
	std::string line = getMessageLine();
	std::string method, resource, major, minor;
	std::istringstream f(line);
	getline(f, method, ' ');
	getline(f, resource, ' ');
	getline(f, major, '/');
	getline(f, major, '.');
	getline(f, minor, '.');
	
	Request req((Method) atoi(method.c_str()), resource, atoi(major.c_str()), atoi(minor.c_str()));
	std::cout << req << std::endl;

	Response res(INSERNAL_SERVER_ERROR, geti("major_version"), geti("minor_version"), *this);
	std::cout << res << std::endl;
	res.doSend(clientSocketFD);
}

std::string Connection::getMessageLine( void )
{
	std::string line = "";
	int received = recv(this->_clientSocket, this->_buffer, sizeof(this->_buffer), 0);
	if (received > 0)
	{
		char c;
		while (this->_buffer[this->_index])
		{
			c = this->_buffer[this->_index];
			if (c == LF && this->_buffer[this->_index + 1] == CR)
				break ;
			line.push_back(c);
			this->_index++;	
		}
	}
	return line;
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

void Connection::ft_error(std::string msg)
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
