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
	std::cout << std::endl << "....... CONFIG ........" << std::endl << std::endl;
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
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

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
			if(setsockopt(this->_serverSocket, SOL_SOCKET, SO_REUSEPORT, &_enable, sizeof(_enable)) == 0) {
				if (connect() == -1)
				{
					/* Handle binding error */
					std::cerr << "[Error] binding client socket" << std::endl;
				}
			}
		}
	}
	else
	{
		/* Handle socket error */
		std::cerr << "[Error] opening server socket" << std::endl;
	}
}

int Connection::connect()
{
	if (bind(this->_serverSocket, (struct sockaddr *) &this->_serverAddress, sizeof(this->_serverAddress)) != -1)
	{
		if (listen(this->_serverSocket, geti("connections")) == 0)
		{
			std::cout << "[Info] server is accepting HTTP connections on port: " << geti("port") << std::endl;
			while (true)
			{
				this->_clientSocket = accept(this->_serverSocket, (struct sockaddr *) &this->_clientAddress, &this->_clientAddressSize);
				if (this->_clientSocket != -1)
					processClientRequest();
				else
				{
					/* Handle accept error */
					std::cerr << "[Error] accepting client connection" << std::endl;
				}
			}
		}
		return 0;
	}
	return -1;
}

void Connection::processClientRequest()
{
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
	res.send();
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

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
