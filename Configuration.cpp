#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration()
{
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Configuration::~Configuration()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Configuration & Configuration::operator=( Configuration const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Configuration const & i )
{
	(void) i;
	o << "Configuration = " << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/
