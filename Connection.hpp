#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <iostream>
# include <string>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <cstring>

class Connection
{

	public:
		Connection(int port);
		~Connection();

		int getPort( void ) const;
		int getServerSocket() const;
		int getClientSocket() const;

	private:
		unsigned int _port;
		int _serverSocket;
		int _clientSocket;
};

std::ostream & operator<<( std::ostream & o, Connection const & i );

#endif
