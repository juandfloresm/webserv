#include "Context.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Context::Context( )
{
	this->_root = "./html";
	this->_autoIndex = false;
}

Context::Context( const Context & c )
{
	*this = c;
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
	this->_index = rhs.getIndex();
	this->_errorPage = rhs.getErrorPage();
	this->_return = rhs.getReturn();
	this->_autoIndex = rhs.getAutoIndex();
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

std::pair<int, std::string> Context::getErrorPage( void ) const
{
	return this->_errorPage;
}

void Context::setErrorPage( int statusCode, std::string page )
{
	this->_errorPage.first = statusCode;
	this->_errorPage.second = page;
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
