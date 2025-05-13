#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <map>
# include <vector>
# include <utility>
# include <cstdlib>
# include <fcntl.h>
# include <unistd.h>
# include "Server.hpp"

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
		bool isEnding(char c);
		bool isTokenValid(int i, std::string token, std::map<int, std::vector<std::string> > levels);
		ServerList & getServerList( void );
		void parseContext( Context & cxt, Entry directive );
	
	private:
		bool _parsingServer;
		ServerList _servers;

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
