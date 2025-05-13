#include "Location.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Location::Location( )
{

}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Location::~Location()
{

}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Location & Location::operator=( Location const & rhs )
{
	this->_root = rhs.getRoot();
	return *this;
}

std::ostream & operator<<( std::ostream & o, Location const & i )
{
	(void) i;
	o << "Location = " << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Location::getPath( void ) const
{
	return this->_path;
}

void Location::setPath( std::string path )
{
	this->_path = path;
}
