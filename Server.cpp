#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Server::Server( )
{
	this->_port = 80;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Server::~Server()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Server & Server::operator=( Server const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Server const & i )
{
	(void) i;
	o << "Server = " << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

int Server::getPort( void ) const
{
	return this->_port;
}

void Server::setPort( int port )
{
	this->_port = port;
}
