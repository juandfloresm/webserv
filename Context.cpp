#include "Context.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Context::Context( )
{
	this->_root = "./html";
	this->_index.push_back("index.html");
	this->_autoIndex = false;
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Context::~Context()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Context & Context::operator=( Context const & rhs )
{
	this->_root = rhs.getRoot();
	return *this;
}

std::ostream & operator<<( std::ostream & o, Context const & i )
{
	(void) i;
	o << "Context = " << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Context::getRoot( void ) const
{
	return this->_root;
}

void Context::setRoot( std::string root )
{
	this->_root = root;
}

std::vector<std::string> Context::getReturn( void ) const
{
	return this->_return;
}

void Context::setReturn( std::string _return )
{
	this->_return.push_back(_return);
}

std::vector<std::string> Context::getIndex( void ) const
{
	return this->_index;
}

void Context::setIndex( std::string index )
{
	this->_return.push_back(index);
}

bool Context::getAutoIndex( void ) const
{
	return this->_autoIndex;
}

void Context::setAutoIndex( bool autoIndex )
{
	this->_autoIndex = autoIndex;
}

/*
** --------------------------------- VIRTUAL ---------------------------------
*/

int Context::getPort( void ) const
{
	return 80;
}

void Context::setPort( int port )
{
	(void) port;
}

std::vector<std::string> Context::getServerNames( void ) const
{
	return std::vector<std::string>();
}

void Context::setServerName( std::string serverName )
{
	(void) serverName;
}

std::string Context::getPath( void ) const
{
	return "/";
}

void Context::setPath( std::string path )
{
	(void) path;
}
