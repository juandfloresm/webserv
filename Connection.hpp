#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <unistd.h>
# include <cstring>
# include <cmath>
# include <sstream>
# include <map>
# include <vector>

# define BUFFER 1024

class Connection
{

	public:
		Connection(std::string config);
		~Connection();

		int getPort( void ) const;
		int getServerSocket() const;
		int getClientSocket() const;

		static void processConfig( Connection & i, std::string line );
		void readFile( std::string file, void (*f)( Connection & i, std::string line ) );

		int get(std::string key) const;

	private:
		unsigned int _port;
		int _serverSocket;
		int _clientSocket;

		std::map<std::string, std::string> _config;

		static const char CONFIG_SEP;
};

std::ostream & operator<<( std::ostream & o, Connection const & i );

#endif
