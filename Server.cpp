#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Server::Server( )
{
	this->_port = 80;
	this->_serverNames.push_back("localhost");
	this->_serverNames.push_back("127.0.0.1");
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
	this->_port = rhs.getPort();
	this->_serverNames = rhs.getServerNames();
	this->_root = rhs.getRoot();
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

std::vector<std::string> Server::getServerNames( void ) const
{
	return this->_serverNames;
}

void Server::setServerName( std::string serverName )
{
	this->_serverNames.push_back(serverName);
}

std::vector<Location> Server::getLocations( void ) const
{
	return this->_locations;
}

void Server::setLocation( Location location )
{
	this->_locations.push_back(location);
}
