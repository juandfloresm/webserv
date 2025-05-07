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
# include "Request.hpp"
# include "Response.hpp"

# define BUFFER 1024
# define LF 10
# define CR 13

typedef std::map<std::string, std::string> Config;

class Connection
{

	public:
		Connection(std::string config);
		~Connection();

		int getPort( void ) const;
		int getServerSocket() const;
		int getClientSocket() const;

		void processConfig(std::string config);
		static void processConfigLine( Connection & i, std::string line );
		void initServer( void );
		int connect();

		void processClientRequest();
		std::string getMessageLine( void );
		
		void readFile( std::string file, void (*f)( Connection & i, std::string line ) );
		int geti(std::string key) const;
		std::string gets(std::string key) const;
		float getf(std::string key) const;

	private:
		unsigned int _port;
		int _serverSocket;
		int _clientSocket;

		sockaddr_in _serverAddress;
		sockaddr_in _clientAddress;
		socklen_t _clientAddressSize;

		Config _config;

		static const char CONFIG_SEP;

		char _buffer[BUFFER];
		int _index;
};

std::ostream & operator<<( std::ostream & o, Connection const & i );

#endif
