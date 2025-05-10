#include "Request.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Request::Request(Method method, std::string resource, int major, int minor, Connection & connection) : Message(major, minor), _method(method), _resource(resource), _connection(connection)
{
	this->_body = "";
	this->_queryString = "";
	std::size_t i = resource.find("?");
  	if (i != std::string::npos)
    {
		this->_resource = resource.substr(0, i);
		this->_queryString = resource.substr(i + 1);
	}
	std::cout << this->_resource << " <> " << this->_queryString << std::endl;
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Request::~Request()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Request & Request::operator=( Request const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Request const & i )
{
	o << std::endl << "....... REQUEST ......." << std::endl << std::endl;
	o << "method = " << i.getMethodString() << std::endl;
	o << "resource = " << i.getResource() << std::endl;
	o << "version = HTTP/" << i.getMajorVersion() << "." << i.getMinorVersion() << std::endl;
	o << std::endl << "......................." << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

const std::string Request::getMethodString( void ) const
{
	switch(this->_method)
	{
		case GET:
			return "GET";
		case POST:
			return "POST";
		case PUT:
			return "PUT";
		case DELETE:
			return "DELETE";
		case TRACE:
			return "TRACE";
		case OPTIONS:
			return "OPTIONS";
		case HEAD:
			return "HEAD";
		default:
			return "UNKNOWN";
	}
}

std::string const Request::header( std::string const header ) const
{
	return this->_connection.getHeaders()[header];
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

Method Request::getMethod( void ) const
{
	return this->_method;
}

std::string Request::getResource( void ) const
{
	return this->_resource;
}

std::string const Request::getQueryString( void ) const
{
	return this->_queryString;
}
