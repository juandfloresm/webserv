#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration( std::string const configFile )
{
	(void) configFile;
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
