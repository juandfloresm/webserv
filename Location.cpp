#include "Location.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Location::Location( )
{
	this->_path = "/";
}

Location::Location( const Location & loc ) : Context(loc)
{
	*this = loc;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Location::~Location() {}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Location & Location::operator=( Location const & rhs )
{
	if (this != &rhs)
	{
		this->_root = rhs.getRoot();
		this->_path = rhs.getPath();
		this->_methods = rhs.getMethods();
		this->_passCGI = rhs.getPassCGI();
		this->_errorPages = rhs.getErrorPages();
		this->_return.first = rhs.getReturn().first;
		this->_return.second = rhs.getReturn().second;
		this->_autoIndex = rhs.getAutoIndex();
		this->_index = rhs.getIndex();
		this->_clientMaxBodySize = rhs.getClientMaxBodySize();
		this->_authBasic = rhs.getAuthBasic();
		this->_mimeTypes = rhs.getMimeTypes();
		this->_uploadPath = rhs.getUploadPath();
	}
	return *this;
}


/*
** --------------------------------- METHODS ----------------------------------
*/


/*
** --------------------------------- ACCESSOR ---------------------------------
*/

std::string Location::getPath( void ) const
{
	return this->_path;
}

void Location::setPath( std::string path )
{
	this->_path = path;
}

std::vector<std::string> Location::getMethods( void ) const
{
	return this->_methods;
}

void Location::setMethod( std::string method )
{
	this->_methods.push_back(method);
}

std::string Location::getPassCGI( void ) const
{
	return this->_passCGI;
}

void Location::setPassCGI( std::string passCGI )
{
	this->_passCGI = passCGI;
}
