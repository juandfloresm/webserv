#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/epoll.h>
# include <unistd.h>
# include <cstring>
# include <cmath>
# include <sstream>
# include <map>
# include <vector>
# include "Request.hpp"
# include "Response.hpp"

# define BUFFER 1024
# define MAX_EVENTS 128
# define MAX_MESSAGE_LEN 512
# define LF 10
# define CR 13
# define CONFIG "./config/zweb.conf"

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
		void preparePolling( void );
		void eventLoop( void );
		void simpleServer( void );

		void processClientRequest( int clientSocketFD );
		std::string getMessageLine( void );
		
		void error(std::string msg);
		std::string gets(std::string key) const;
		int geti(std::string key) const;
		float getf(std::string key) const;
		void readFile( std::string file, void (*f)( Connection & i, std::string line ) );

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

		struct epoll_event _pollEvent;
		struct epoll_event _events[MAX_EVENTS];
		int _epollfd;
};

std::ostream & operator<<( std::ostream & o, Connection const & i );

#endif
