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
		Location();
		Location( const Location & loc );
		~Location();
		Location & operator=( Location const & rhs );

		std::string getPath( void ) const;
		void setPath( std::string path );
		std::vector<std::string> getMethods( void ) const;
		void setMethod( std::string method );
		std::string getPassCGI( void ) const;
		void setPassCGI( std::string passGCI );

	private:
		std::string _path;
		std::vector<std::string> _methods;
		std::string _passCGI;

};

std::ostream & operator<<( std::ostream & o, Location const & i );

#endif
