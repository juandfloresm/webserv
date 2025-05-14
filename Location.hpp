#ifndef LOCATION_HPP
# define LOCATION_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include "Context.hpp"

class Location : public Context
{

	public:
		Location( );
		~Location();
		Location & operator=( Location const & rhs );

		std::string getPath( void ) const;
		void setPath( std::string path );

	private:
		std::string _path;
		
		// NOT WIRED
		std::vector<std::string> _methods;

};

std::ostream & operator<<( std::ostream & o, Location const & i );

#endif
