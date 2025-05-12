#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <utility>
# include <cstdlib>
# include <fcntl.h>
# include <unistd.h>
# include "./Server.hpp"

typedef std::pair<std::string, std::string> Entry;

class Configuration
{

	public:
		Configuration( std::string const configFile );
		~Configuration();
		Configuration & operator=( Configuration const & rhs );

		void parse( std::string const file );
		void parseEntry( int level, Entry directive );
		bool isSpace(char c);
		bool isEnding(char c);
		bool isTokenValid(int i, std::string token, std::map<int, std::vector<std::string> > levels);
	
	private:
		std::string _pad;
		bool _parsingServer;
		Server _server;
		std::vector<Server> _servers;

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
