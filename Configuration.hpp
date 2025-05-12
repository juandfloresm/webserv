#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include <fcntl.h>
# include <unistd.h>

class Configuration
{

	public:
		Configuration( std::string const configFile );
		~Configuration();
		Configuration & operator=( Configuration const & rhs );

		void parse( std::string const file );
		void parseDirective( std::string directive );
		bool isSpace(char c);
		bool isEnding(char c);
		bool isTokenValid(int i, std::string token, std::map<int, std::vector<std::string> > levels);
	
	private:
		std::string _pad;
		bool _parsingServer;
		bool _parsingLocation;

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
