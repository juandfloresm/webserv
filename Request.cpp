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
	} catch ( Response::UnsupportedMediaTypeException & e ) {
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
	parseContentType();
	if (isFormContentType() && _method == POST)
	{
		if (eq(header(TRANSFER_ENCODING), CHUNKED))
		{
			unsigned long n = 0;
			unsigned char c = '\0', pc = '\0';
			std::string hx = "";
			unsigned char buffer[BUFFER];
			size_t bufferSize = BUFFER;
			std::string base = "0123456789abcdefABCDEF";
			unsigned int counter = 0;
			while(read(_clientSocket, buffer, bufferSize) > 0)
			{
				c = (unsigned char) buffer[0];
				if (pc == CR && c == LF)
				{
					if (hx.empty())
					{
						counter++;
						if (counter == 2)
							break;
					}
					else
					{
						std::stringstream ss;
						ss << std::hex << hx;
						ss >> n;
						_contentLength += n;
						parseContentFragment(clientMaxBodySize, n);
						hx = "";
						n = 0;
					}
				}
				else if (base.find(c) != std::string::npos)
					hx.push_back(c);
				pc = c;
			}
		}
		else
		{
			parseContentLength();
			parseContentFragment(clientMaxBodySize, _contentLength);
		}

		if (_contentType.compare(FORM_TYPE_MULTIPART) == 0)
		{
			parseMultipartContent();
		}
		else if (_contentType.compare(FORM_TYPE_PLAIN) == 0)
		{
			throw Response::UnprocessableContentException();
		}
		else if (_contentType.compare(FORM_TYPE_APPLICATION) == 0)
		{
			throw Response::UnprocessableContentException();
		}
	}
}

void Request::parseContentFragment( unsigned long max, unsigned long n )
{
	if ( max > 0 && max < _contentLength )
		throw Response::ContentTooLargeException();

	unsigned char buffer[BUFFER];
	size_t bufferSize = BUFFER;

	for (unsigned long i = 0; i < n; i++)
	{
		if (read(_clientSocket, buffer, bufferSize) > 0)
			_body.push_back(buffer[0]);
	}
}

void Request::setPart(std::string & name, std::string & value)
{
	if (value.empty())
		throw Response::UnprocessableContentException();
	if (name.find(";") != std::string::npos)
	{
		std::string nm = name.substr(6, name.find(";") - 1 - 6);
		std::string file = name.substr(name.find(";") + 12, name.size() - name.find(";") + 12);
		file = file.substr(0, file.size() - 1);
		_content["filename"] = file;
		_content[nm] = value;
		std::string path = std::getenv("WPATH");
		path +=  ("/html/uploads/" + file);
		std::ofstream MyFile(path.c_str());
		MyFile << _content[nm];
		MyFile.close();
	}
	else
	{
		std::string nm = name.substr(6, name.rfind("\"") - 6);
		_content[nm] = value;
	}
}

void Request::parseMultipartContent( void )
{
	bool emptyLine = false;
	std::string line, name, type, value;
	std::string delimiter = _boundary;
	std::string liner = "\r\n";
	std::vector<std::string> parts = split(_body, delimiter), lines;
	for(size_t i = 1; i < parts.size(); i++)
	{
		lines = split(parts[i], liner);

		if (lines.size() < 4) {
			throw Response::UnprocessableContentException();
		}

		line = lines.at(1);							//.....................................
		if (line.find(CONTENT_DISPOSITION) != 0)
			throw Response::UnprocessableContentException();
		else
			name = line.substr(line.find("name"));

		line = lines.at(2);							//.....................................
		if (line.empty())
			emptyLine = true;
		else if (line.find(CONTENT_TYPE) == 0)
		{
			type = line;
			std::istringstream f(type);
			std::string s;
			getline(f, s, ' ');
			getline(f, type, ' ');
			if (eq(type, "image/png") || eq(type, "image/gif") || eq(type, "image/jpg") || \
				eq(type, "image/jpeg") || eq(type, "application/pdf") || \
				eq(type, "text/plain") || eq(type, "text/html"))
			{
				// TODO: delimit supporting MIME types from config file
			}
			else
				throw Response::UnsupportedMediaTypeException();

		}
		else
			throw Response::UnprocessableContentException();

		line = lines.at(3);							//.....................................
		if (emptyLine)
		{
			value = line;
			setPart(name, value);
			emptyLine = false;
			continue;
		}
		else if (line.empty())
		{
			emptyLine = true;
			if (lines.size() < 6) {
				throw Response::UnprocessableContentException();
			}
			line = lines.at(4);
			value = line + liner + lines.at(5);
			setPart(name, value);
			emptyLine = false;
			continue;
		}
		else
			throw Response::UnprocessableContentException();
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
	std::string len = header(CONTENT_LENGTH);

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
	std::string raw = header(CONTENT_TYPE);
	std::string len = header(CONTENT_LENGTH);
	if (!len.empty() && raw.empty())
		throw Response::UnsupportedMediaTypeException();
	if (!raw.empty())
	{
		std::istringstream f(raw);
		std::string s, h, b;
		std::getline(f, s);
		std::istringstream r(s);
		std::getline(r, h, ';');
		std::getline(r, b, ';');
		_contentType = h;

		if (_contentType.compare(FORM_TYPE_MULTIPART) == 0)
		{
			if (b.find("boundary=") == 1)
			{
				_boundary = b.substr(10);
				if (_boundary.size() > 0)
					return;
			}
			throw Response::UnsupportedMediaTypeException();
		}
		else
		{
			if (b.find("charset=") == 1)
			{
				_charSet = b.substr(9);
				if (_charSet.compare("UTF-8") != 0)
					throw Response::UnsupportedMediaTypeException();
			}
		}
	}
}

bool Request::isContentAvailable( void ) const
{
	return _body.size() > 0;
}

bool Request::isFormContentType( void )
{
	return 	_contentType.compare(FORM_TYPE_MULTIPART) == 0 || \
			_contentType.compare(FORM_TYPE_APPLICATION) == 0|| \
			_contentType.compare(FORM_TYPE_PLAIN) == 0;
}

std::vector<std::string> Request::split(std::string& s, std::string& delimiter)
{
    std::vector<std::string> tokens;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        tokens.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    return tokens;
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
