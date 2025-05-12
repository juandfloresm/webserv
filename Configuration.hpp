#ifndef CONFIGURATION_HPP
# define CONFIGURATION_HPP

# include <iostream>
# include <string>
# include <map>

class Configuration
{

	public:
		Configuration();
		~Configuration();
		Configuration & operator=( Configuration const & rhs );
	
	protected:

};

std::ostream & operator<<( std::ostream & o, Configuration const & i );

#endif
