#include "Server.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Server::Server( )
{
	this->_port = 80;
	this->_default = false;
	this->_serverNames.push_back("localhost");
}

Server::Server( const Server & server ) : Context(server)
{
	*this = server;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Server::~Server() {}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Server & Server::operator=( Server const & rhs )
{
	this->_port = rhs.getPort();
	this->_serverNames = rhs.getServerNames();
	this->_index = rhs.getIndex();
	this->_root = rhs.getRoot();
	this->_errorPages = rhs.getErrorPages();
	this->_return.first = rhs.getReturn().first;
	this->_return.second = rhs.getReturn().second;
	this->_autoIndex = rhs.getAutoIndex();
	this->_locations = rhs.getLocations();
	this->_clientMaxBodySize = rhs.getClientMaxBodySize();
	return *this;
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

Location & Server::getLastLocation( void )
{
	return this->_locations.back();
}

void Server::setLocation( Location location )
{
	this->_locations.push_back(location);
}

bool Server::isDefault( void ) const
{
	return this->_default;
}

void Server::setDefault( bool _default )
{
	this->_default = _default;
}
