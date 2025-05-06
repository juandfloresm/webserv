#include "Message.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Message::Message(int major, int minor) : _majorVersion(major), _minorVersion(minor)
{
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Message::~Message()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Message & Message::operator=( Message const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Message const & i )
{
	o << "Message = " << i.getMajorVersion() << "." << i.getMinorVersion() << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/
int Message::getMajorVersion( void ) const
{
	return this->_majorVersion;
}

int Message::getMinorVersion( void ) const
{
	return this->_minorVersion;
}
