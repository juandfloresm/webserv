#include "Connection.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Connection::Connection(int port) : _port(port)
{
	this->_serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (this->_serverSocket != -1)
	{
		sockaddr_in addr;
		sockaddr_in client;

		memset(&addr, '\0', sizeof(sockaddr_in));
		addr.sin_family = AF_INET;
		addr.sin_port = htons(port);
		addr.sin_addr.s_addr = INADDR_ANY;
		socklen_t size = sizeof(client);
			
		if (bind(this->_serverSocket, (struct sockaddr *) &addr, sizeof(addr)) != -1)
		{
			if (listen(this->_serverSocket, 10) == 0)
			{
				std::cerr << "[Info] server is accepting HTTP connections on port: " << port << std::endl;
				while (true)
				{
					this->_clientSocket = accept(this->_serverSocket, (struct sockaddr *) &client, &size);
					if (this->_clientSocket != -1)
					{
						char buffer[1024] = { 0 };
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
