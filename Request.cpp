#include "Request.hpp"

const char Request::HEADER_SEP = ':';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/
Request::Request(int clientSocket) : Message(), _clientSocket(clientSocket)
{
	this->_body = "";
	this->_queryString = "";
	parseRequest();
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

void Request::parseRequest( void )
{
	std::string line = getMessageLine();
	std::string method, resource, major, minor;
	std::istringstream f(line);

	getline(f, method, ' ');
	if (method.find("GET") != std::string::npos)
		this->_method = GET;
	else if (method.find("POST") != std::string::npos)
		this->_method = POST;
	else if (method.find("DELETE") != std::string::npos)
		this->_method = DELETE;
	else
	{
		this->_method = UNKNOWN;
		return;
	}
	
	getline(f, resource, ' ');
	this->_resource = resource;
	std::size_t i = resource.find("?");
  	if (i != std::string::npos)
    {
		this->_resource = resource.substr(0, i);
		this->_queryString = resource.substr(i + 1);
	}

	getline(f, major, '/');
	getline(f, major, '.');
	getline(f, minor, '.');	
	setMajorVersion(atoi(major.c_str()));
	setMinorVersion(atoi(minor.c_str()));

	parseHeaders();
}

std::string Request::header( std::string const header )
{
	HeaderIterator it = this->_headers.find(header);
	if (it != this->_headers.end())
		return it->second;
	else
		return "";
}

void Request::parseHeaders( void )
{
	std::string line = "";
	this->_headers.clear();
	while (!(line = getMessageLine()).empty())
	{
		std::string key, value;
		std::istringstream f(line);
		getline(f, key, Request::HEADER_SEP);
		getline(f, value, Request::HEADER_SEP);
		std::string v = "";
		for (size_t i = 0; i < value.size(); i++)
		{
			if (value[i] == ' ')
				continue;
			v.push_back(value[i]);
		}
		this->_headers[key] = v;
	}
}

std::string Request::getMessageLine( void )
{
	std::string line = "";
	char c = '\0', p = '\0';
	while (recv(this->_clientSocket, this->_buffer, 1, 0) > 0)
	{
		c = this->_buffer[0];
		if (c == LF || c == CR)
		{
			if (p == CR)
				break;
			else
			{
				p = c;
				continue;
			}
		}
		line.push_back(c);
	}
	return line;
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

Header & Request::getHeaders( void )
{
	return this->_headers;
}
