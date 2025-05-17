#include "Request.hpp"

const char Request::HEADER_SEP = ':';

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/
Request::Request(int clientSocket, Configuration & cfg, int port) : Message(clientSocket, cfg)
{
	this->_body = "";
	this->_resource = "";
	this->_queryString = "";
	this->_contentType = "";
	this->_charSet = "";
	this->_boundary = "";
	this->_contentLength = 0;

	try {
		parseTopLine();
		parseHeaders();
		Response(OK, clientSocket, cfg, port, *this);
	} catch ( Response::LengthRequiredException & e ) {
		Response(LENGTH_REQUIRED, clientSocket, cfg, port, *this);
	} catch ( Response::UnsuportedMediaTypeException & e ) {
		Response(UNSUPPORTED_MEDIA_TYPE, clientSocket, cfg, port, *this);
	} catch ( Response::UnprocessableContentException & e ) {
		Response(UNPROCESSABLE_CONTENT, clientSocket, cfg, port, *this);
	} catch ( Response::MethodNotAllowedException & e ) {
		Response(METHOD_NOT_ALLOWED, clientSocket, cfg, port, *this);
	} catch ( Response::ContentTooLargeException & e ) {
		Response(CONTENT_TOO_LARGE, clientSocket, cfg, port, *this);
	} catch ( Response::InternalServerException & e ) {
		Response(INTERNAL_SERVER_ERROR, clientSocket, cfg, port, *this);
	} catch ( Response::NotImplementedException & e ) {
		Response(NOT_IMPLEMENTED, clientSocket, cfg, port, *this);
	}
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Request::~Request(){}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

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
		case DELETE:
			return "DELETE";
		default:
			return "UNKNOWN";
	}
}

void Request::parseTopLine( void )
{
	std::string line = getMessageLine();
	std::string method, resource, major, minor;
	std::istringstream f(line);

	getline(f, method, ' ');

	if (eq(method, "GET"))
		this->_method = GET;
	else if (eq(method, "POST"))
		this->_method = POST;
	else if (eq(method, "DELETE"))
		this->_method = DELETE;
	else if (eq(method, "HEAD") || \
			 eq(method, "PUT") || \
			 eq(method, "CONNECT") || \
			 eq(method, "OPTIONS") || \
			 eq(method, "TRACE") || \
			 eq(method, "PATCH"))
		throw Response::MethodNotAllowedException();
	else
		throw Response::NotImplementedException();
	
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
		bool init = false;
		for (size_t i = 0; i < value.size(); i++)
		{
			if (value[i] == ' ' && !init)
				continue;
			init = true;
			v.push_back(value[i]);
		}
		this->_headers[key] = v;
	}
}

void Request::parseContent( unsigned long clientMaxBodySize )
{
	unsigned char buffer[BUFFER];
	size_t bufferSize = BUFFER;

	parseContentType();
	if (isFormContentType() && _method == POST)
	{
		parseContentLength();
		if ( clientMaxBodySize > 0 && clientMaxBodySize < _contentLength )
			throw Response::ContentTooLargeException();

		for (unsigned long i = 0; i < _contentLength; i++)
		{
			if (read(_clientSocket, buffer, bufferSize) > 0)
				_body.push_back(buffer[0]);
		}
	}
}

/*
** --------------------------------- UTILITIES ---------------------------------
*/

bool Request::eq( std::string s1, std::string s2 )
{
	return (s1.compare(s2) == 0);
}

void Request::parseContentLength( void )
{
	std::string len = header("Content-Length");

	if (len.empty())
		throw Response::LengthRequiredException();

	try {
		_contentLength = _cfg.number(len);
	} catch (std::runtime_error & e) {
		throw Response::UnprocessableContentException();
	}
}

void Request::parseContentType( void )
{
	std::string raw = header("Content-Type");
	std::string len = header("Content-Length");
	if (!len.empty() && raw.empty())
		throw Response::UnsuportedMediaTypeException();
	if (!raw.empty())
	{
		std::istringstream f(raw);
		std::string h, b;
		getline(f, h, ';');
		getline(f, b, ';');
		_contentType = h;
		_boundary = b;
		_charSet = b;
	}
}

bool Request::isContentAvailable( void ) const
{
	return _body.size() > 0;
}

bool Request::isFormContentType( void )
{
	return 	_contentType.compare("multipart/form-data") == 0 || \
			_contentType.compare("application/x-www-form-urlencoded") == 0|| \
			_contentType.compare("text/plain") == 0;
}

std::string Request::getMessageLine( void )
{
	std::string line = "";
	char c = '\0', p = '\0';
	while (recv(_clientSocket, this->_buffer, 1, 0) > 0)
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

std::string Request::header( std::string const header )
{
	HeaderIterator it = this->_headers.find(header);
	if (it != this->_headers.end())
		return it->second;
	else
		return "";
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

void Request::setResource( std::string resource )
{
	this->_resource = resource;
}

std::string const Request::getQueryString( void ) const
{
	return this->_queryString;
}

Header & Request::getHeaders( void )
{
	return this->_headers;
}
