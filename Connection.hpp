#ifndef CONNECTION_HPP
# define CONNECTION_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <sstream>
# include <map>
# include <vector>

# include <cstring>
# include <cmath>
# include <sys/socket.h>
# include <netinet/in.h>
# include <sys/epoll.h>
# include <unistd.h>

# include "Configuration.hpp"
# include "Request.hpp"
# include "Response.hpp"

# define MAX_EVENTS 128
# define MIN_CONNECTIONS 10

typedef std::map<std::string, std::string> Config;
typedef struct epoll_event EpollEvent;

typedef struct _data
{
	Config config;
	EpollEvent pollEvent;
	int serverSocket;
	int clientSocket;
	int port;
} SocketData;

typedef std::map<int, SocketData> SocketDataEvents;
typedef SocketDataEvents::iterator SocketDataEventsIterator;
typedef std::map<std::string, std::string> Session;
typedef std::map<std::string, Session> Sessions;

class Connection
{

	public:
		Connection( Configuration & cfg );
		~Connection();

		void initServer( void );
		int connect( int serverSocket, int port, int connections );
		void initServers( void );
		void eventLoop( void );

		void processClientRequest( int clientSocketFD );
		
		void ft_error(const std::string msg) const;
		static void handleSigint( int sgn );
		Configuration & getConfiguration( void ) const;

		std::vector<std::string> & getSession( void );

	private:
		unsigned int _port;

		Config _config;
		Configuration & _cfg;
		static const char CONFIG_SEP;

		SocketDataEvents _serverEvents;
		SocketDataEvents _clientEvents;
		struct epoll_event _events[MAX_EVENTS];
		int _epollfd;

		Sessions _sessions;
};

#endif
