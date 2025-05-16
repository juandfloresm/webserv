#include "Message.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Message::Message(int socket, Configuration & cfg) : _cfg(cfg), _clientSocket(socket)
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

void Message::setMajorVersion( int major )
{
	this->_majorVersion = major;
}

void Message::setMinorVersion( int minor )
{
	this->_minorVersion = minor;
}
