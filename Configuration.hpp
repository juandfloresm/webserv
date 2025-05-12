#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <map>

class Configuration
{

	public:
		Configuration( std::string const configFile  );
		~Configuration();
		Configuration & operator=( Configuration const & rhs );
	
	protected:

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
