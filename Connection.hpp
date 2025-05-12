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

# define MAX_EVENTS 128
# define MAX_MESSAGE_LEN 512
# define CONFIG "./config/zweb.conf"

typedef std::map<std::string, std::string> Config;
typedef struct epoll_event EpollEvent;

typedef struct _data
{
	Config config;
	EpollEvent pollEvent;
	int serverSocket;
	int clientSocket;
} SocketData;

typedef std::map<int, SocketData> SocketDataEvents;
typedef SocketDataEvents::iterator SocketDataEventsIterator;

class Request;
class Response;

class Connection
{

	public:
		Connection(std::string config);
		~Connection();

		void processConfig(std::string config);
		static void processConfigLine( Connection & i, std::string line );
		void initServer( void );
		int connect( int serverSocket, Config config );
		void initServers( void );
		void eventLoop( void );

		void processClientRequest( int clientSocketFD );
		
		void ft_error(const std::string msg) const;
		std::string gets(Config m, std::string key) const;
		int geti(Config m, std::string key) const;
		float getf(Config m, std::string key) const;
		void readFile( std::string file, void (*f)( Connection & i, std::string line ) );
		static void handleSigint( int sgn );

	private:
		unsigned int _port;

		Config _config;
		static const char CONFIG_SEP;

		SocketDataEvents _serverEvents;
		SocketDataEvents _clientEvents;
		struct epoll_event _events[MAX_EVENTS];
		int _epollfd;

};

std::ostream & operator<<( std::ostream & o, Connection const & i );

#endif
