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

		std::string parse( std::string const file );
		void parseDirective( std::string directive );
		bool isSpace(char c);
		bool isTokenValid(int i, std::string token, std::map<int, std::vector<std::string> > levels);
	
	private:
		std::string _pad;

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
