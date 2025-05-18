#include "Context.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Context::Context( )
{
	this->_root = "./html";
	this->_autoIndex = false;
	this->_clientMaxBodySize = 0;
	this->_authBasic = "";
}

Context::Context( const Context & c )
{
	*this = c;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Context::~Context() {}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Context & Context::operator=( Context const & rhs )
{
	if (this != &rhs)
	{
		this->_root = rhs.getRoot();
		this->_index = rhs.getIndex();
		this->_errorPages = rhs.getErrorPages();
		this->_return = rhs.getReturn();
		this->_autoIndex = rhs.getAutoIndex();
		this->_clientMaxBodySize = rhs.getClientMaxBodySize();
		this->_authBasic = rhs.getAuthBasic();
	}
	return *this;
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

std::vector<std::pair<int, std::string> > Context::getErrorPages( void ) const
{
	return this->_errorPages;
}

void Context::setErrorPage( int statusCode, std::string page )
{
	std::pair<int, std::string> pair;
	pair.first = statusCode;
	pair.second = page;
	this->_errorPages.push_back(pair);
}

std::pair<int, std::string> Context::getReturn( void ) const
{
	return this->_return;
}

void Context::setReturn( int statusCode, std::string page )
{
	this->_return.first = statusCode;
	this->_return.second = page;
}

std::vector<std::string> Context::getIndex( void ) const
{
	return this->_index;
}

void Context::setIndex( std::string index )
{
	this->_index.push_back(index);
}

bool Context::getAutoIndex( void ) const
{
	return this->_autoIndex;
}

void Context::setAutoIndex( bool autoIndex )
{
	this->_autoIndex = autoIndex;
}

unsigned long Context::getClientMaxBodySize( void ) const
{
	return this->_clientMaxBodySize;
}

void Context::setClientMaxBodySize( unsigned long sz )
{
	this->_clientMaxBodySize = sz;
}

std::string Context::getAuthBasic( void ) const
{
	return _authBasic;
}

void Context::setAuthBasic( std::string authBasic )
{
	_authBasic = authBasic;
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

std::string Context::getPath( void ) const
{
	return "/";
}

void Context::setPath( std::string path )
{
	(void) path;
}

std::vector<std::string> Context::getMethods( void ) const
{
	return std::vector<std::string>();
}

void Context::setMethod( std::string method )
{
	(void) method;
}

std::string Context::getPassCGI( void ) const
{
	return "";
}

void Context::setPassCGI( std::string passCGI )
{
	(void) passCGI;
}
