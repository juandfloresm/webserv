#include "Connection.hpp"

const char Connection::CONFIG_SEP = '=';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Connection::Connection(std::string config)
{
	std::cout << std::endl << "....... CONFIG ........" << std::endl << std::endl;
	readFile(config, Connection::processConfig);
	std::cout << std::endl << "......................." << std::endl << std::endl;

	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->_serverSocket != -1)
	{
		sockaddr_in addr;
		sockaddr_in client;

		int port = geti("port");

		memset(&addr, '\0', sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		socklen_t size = sizeof(client);
			
		if (bind(this->_serverSocket, (struct sockaddr *) &addr, sizeof(addr)) != -1)
		{
			if (listen(this->_serverSocket, geti("connections")) == 0)
			{
				std::cerr << "[Info] server is accepting HTTP connections on port: " << port << std::endl;
				while (true)
				{
					this->_clientSocket = accept(this->_serverSocket, (struct sockaddr *) &client, &size);
					if (this->_clientSocket != -1)
						processClientRequest();
					else
					{
						std::cerr << "[Error] accepting client connection" << std::endl;
						break;
					}

					std::cout << std::endl << std::endl << std::endl << std::endl;
				}
			}
		}
		else
		{
			/* Handle binding error */
			std::cerr << "[Error] binding client socket" << std::endl;
		}
	}
	else
	{
		/* Handle socket error */
		std::cerr << "[Error] opening server socket" << std::endl;
	}
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

std::string Connection::gets(std::string key) const
{
	std::map<std::string, std::string> m = this->_config;
	if (m.find(key) == m.end()) {
		return "";
	} else {
		return m[key];
	}
}

int Connection::geti(std::string key) const
{
	std::map<std::string, std::string> m = this->_config;
	if (m.find(key) == m.end()) {
		return 0;
	} else {
		return atoi(m[key].c_str());
	}
}

float Connection::getf(std::string key) const
{
	std::map<std::string, std::string> m = this->_config;
	if (m.find(key) == m.end()) {
		return 0;
	} else {
		return atof(m[key].c_str());
	}
}

void Connection::processConfig( Connection & i, std::string line )
{
	std::string key, value;
	std::istringstream f(line);
	getline(f, key, Connection::CONFIG_SEP);
	getline(f, value, Connection::CONFIG_SEP);
	std::cout << key << ": " << value << std::endl;
	i._config[key] = value;
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

void Connection::processClientRequest()
{
	this->_index = 0;
	std::string line = getLine();
	std::string method, resource, major, minor;
	std::istringstream f(line);
	getline(f, method, ' ');
	getline(f, resource, ' ');
	getline(f, major, '/');
	getline(f, major, '.');
	getline(f, minor, '.');
	Request r((Method) atoi(method.c_str()), resource, atoi(major.c_str()), atoi(minor.c_str()));
	std::cout << r << std::endl;
}

std::string Connection::getLine( void )
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
