#include "Response.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Response::Response(Status status, int major, int minor, const Connection & connection, const Request & request) : Message(major, minor), _status(status), _connection(connection), _request(request)
{
	initStatusDescriptions();
	sampleResonseSetup();
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Response::~Response()
{
}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Response & Response::operator=( Response const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Response const & i )
{
	o << std::endl << "....... RESPONSE ......" << std::endl << std::endl;
	o << i.toString();
	o << "......................." << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

void Response::sampleResonseSetup( void )
{
	std::ostringstream ss;
    ss << this->_status;
	this->_description = this->_statusDescriptions[this->_status];
	this->_statusString = ss.str();
	this->_content = readPage();
	this->_contentLength = this->_content.size();
}

void Response::doSend( int fd )
{
	std::string resp = toString();
	send(fd, resp.c_str(), resp.size(), 0);
}

void Response::initStatusDescriptions( void )
{
	/* 200 - 299 .............................. */
	this->_statusDescriptions[OK] = "OK";

	/* 300 - 399 .............................. */
	this->_statusDescriptions[MOVED_PERMANENTLY] = "Moved Permanently";

	/* 400 - 499 .............................. */
	this->_statusDescriptions[NOT_FOUND] = "Not Found";

	/* 500 - 599 .............................. */
	this->_statusDescriptions[INSERNAL_SERVER_ERROR] = "Internal Server Error";
}

const std::string Response::toString( void ) const
{
	std::ostringstream ss;
    ss << getMajorVersion();
	std::string major = ss.str();
	ss << getMinorVersion();
	std::string minor = ss.str();
	std::string contentType = "text/html";

	ss.str("");
	ss.clear();
	ss << this->_contentLength;
	std::string contentLength = ss.str();

	std::string r = "";

	// Version
	r += "HTTP/" + major + "." + minor + " ";
	
	// Status
	r += this->_statusString + " " + getDescription() + CRLF;

	// Headers
	r += "Content-Type: " + contentType + CRLF;
	r += "Content-Length: " + contentLength + CRLF;
	r += CRLF; // Empty line between headers and content

	r += this->_content;
	r += CRLF;

	return r;
}

std::string Response::readError( std::string status ) const
{
	std::string line;
	std::string filePath = this->_connection.gets("error_pages") + '/' + status + ".html";
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "[Error] No error file match " << filePath << std::endl;
		return "";
	}
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

std::string Response::readPage( void ) const
{
	std::string base = this->_connection.gets("static_route");
	std::string path = this->_request.getResource();

	if (path.empty() || path[0] != '/')
		path = "/" + path;
	
	bool isDirectory = (path == "/" || (path.length() > 0 && path[path.length() - 1] == '/'));
	if (isDirectory)
		path += "index.html"; // Default file

	std::string filePath = base + path;
	std::cout << "Serving file: " << filePath << std::endl;


	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "[Error] No error file match " << filePath << std::endl;
		return readError("404");
	}

	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

Status Response::getStatus( void ) const
{
	return this->_status;
}

std::string Response::getDescription( void ) const
{
	return this->_description;
}
