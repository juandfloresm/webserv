#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <map>
# include <vector>
# include <utility>
# include <cstdlib>
# include <cctype>
# include <fcntl.h>
# include <unistd.h>
# include "Server.hpp"
# include "Location.hpp"

typedef std::pair<std::string, std::string> Entry;
typedef std::vector<Server> ServerList;
typedef ServerList::iterator ServerListIterator;

class Configuration
{

	public:
		Configuration( std::string const configFile );
		~Configuration();
		Configuration & operator=( Configuration const & rhs );

		void parse( std::string const file );
		void parseEntry( Entry directive );
		bool isSpace(char c);
		bool isEnding(char c, int * i);
		bool isTokenValid(int i, std::string token);
		ServerList & getServerList( void );
		void parseContext( Context & cxt, Entry directive );
		void processToken( int fd, std::string token, int * i );
		int port( std::string raw );
		int statusCode( std::string raw );
		std::string word( std::string raw );
		std::string methods( std::string raw );
		std::string flag( std::string raw );
	
	private:
		bool _parsingServer;
		ServerList _servers;
		Location _location;

		std::map<int, std::vector<std::string> > levels;
		std::vector<std::string> level0, level1, level2;
};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
