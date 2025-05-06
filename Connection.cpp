#include "Connection.hpp"

const char Connection::CONFIG_SEP = '=';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Connection::Connection(std::string config)
{

	readFile(config, Connection::processConfig);

	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->_serverSocket != -1)
	{
		sockaddr_in addr;
		sockaddr_in client;

		int port = get("port");

		memset(&addr, '\0', sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		socklen_t size = sizeof(client);
			
		if (bind(this->_serverSocket, (struct sockaddr *) &addr, sizeof(addr)) != -1)
		{
			if (listen(this->_serverSocket, get("connections")) == 0)
			{
				std::cerr << "[Info] server is accepting HTTP connections on port: " << port << std::endl;
				while (true)
				{
					this->_clientSocket = accept(this->_serverSocket, (struct sockaddr *) &client, &size);
					if (this->_clientSocket != -1)
					{
						char buffer[BUFFER];
						int received = recv(this->_clientSocket, buffer, sizeof(buffer), 0);
						if (received > 0)
							std::cout << buffer << std::endl;
					}
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

int Connection::get(std::string key) const
{
	std::map<std::string, std::string> m = this->_config;
	if (m.find("f") == m.end()) {
		return (-1);
	} else {
		return atoi(m[key].c_str());
	}
}

void Connection::processConfig( Connection & i, std::string line )
{
	std::string key, value;
	std::istringstream f(line);
	getline(f, key, Connection::CONFIG_SEP);
	getline(f, value, Connection::CONFIG_SEP);
	std::cout << "Key: " << key << ", Value: " << value << std::endl;
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
