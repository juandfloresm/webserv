#include "Response.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Response::Response(Status status, int clientSocket, const Connection & connection, Request & request) : Message(), _status(status), _connection(connection), _request(request), _clientSocket(clientSocket)
{
	setMajorVersion(connection.geti("major_version"));
	setMinorVersion(connection.geti("minor_version"));
	initStatusDescriptions();
	this->_headerSection = "";
	if (status == OK)
		doResponse();
	else
		doSend(clientSocket);
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

void Response::doResponse( void )
{
	if (this->_request.getResource().find(".php") != std::string::npos)
		this->_content = readDynamicPage();
	else
		this->_content = readStaticPage();
	doSend(this->_clientSocket);
}

void Response::doSend( int fd )
{
	this->_contentLength = this->_content.size();
	std::ostringstream ss;
	ss << this->_status;
	this->_description = this->_statusDescriptions[this->_status];
	this->_statusString = ss.str();
	std::string resp = toString();
	send(fd, resp.c_str(), resp.size(), 0);
}

void Response::initStatusDescriptions( void )
{
	/* 100 - 199 (Informational) */
	this->_statusDescriptions[CONTINUE] = "Continue";
	this->_statusDescriptions[SWITCHING_PROTOCOLOS] = "Switching Protocols";

	/* 200 - 299 .............................. */
	this->_statusDescriptions[OK] = "OK";
	this->_statusDescriptions[CREATED] = "Created";
	this->_statusDescriptions[ACCEPTED] = "Accepted";
	this->_statusDescriptions[NO_CONTENT] = "No Content";
	this->_statusDescriptions[PARTIAL_CONTENT] = "Partial Content";

	/* 300 - 399 .............................. */
	this->_statusDescriptions[MULTIPLE_CHOICES] = "Multiple Choices";
	this->_statusDescriptions[MOVED_PERMANENTLY] = "Moved Permanently";
	this->_statusDescriptions[FOUND] = "Found";
	this->_statusDescriptions[SEE_OTHER] = "See Other";
	this->_statusDescriptions[NOT_MODIFIED] = "Not Modified";
	this->_statusDescriptions[TEMPORARY_REDIRECT] = "Temporary Redirect";
	this->_statusDescriptions[PERMANENET_REDIRECT] = "Permanent Redirect";

	/* 400 - 499 .............................. */
	this->_statusDescriptions[BAD_REQUEST] = "Bad Request";
	this->_statusDescriptions[UNAUTHORIZED] = "Unauthorized";
	this->_statusDescriptions[FORBIDDEN] = "Forbidden";
	this->_statusDescriptions[NOT_FOUND] = "Not Found";
	this->_statusDescriptions[METHOD_NOT_ALLOWED] = "Method Not Allowed";
	this->_statusDescriptions[NOT_ACCEPTABLE] = "Not Acceptable";
	this->_statusDescriptions[REQUEST_TIMEOUT] = "Request Timeout";
	this->_statusDescriptions[CONFLICT] = "Conlifct";
	this->_statusDescriptions[GONE] = "Gone";
	this->_statusDescriptions[LENGTH_REQUIRED] = "Length Required";
	this->_statusDescriptions[PAYLOAD_TOO_LARGE] = "Payload Too Large";
	this->_statusDescriptions[URI_TOO_LONG] = "URI Too Long";
	this->_statusDescriptions[UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
	this->_statusDescriptions[EXPECTATION_FAILED] = "Expectation Failed";
	this->_statusDescriptions[TOO_MANY_REQUESTS] = "Too Many Requests";

	/* 500 - 599 .............................. */
	this->_statusDescriptions[INTERNAL_SERVER_ERROR] = "Internal Server Error";
	this->_statusDescriptions[NOT_IMPLEMENTED] = "Not Implemented";
	this->_statusDescriptions[BAD_GATEWAY] = "Bad Gateway";
	this->_statusDescriptions[SERVICE_UNAVAILABLE] = "Service Unavailable";
	this->_statusDescriptions[GATEWAY_TIMEOUT] = "Gateway Timeout";
	this->_statusDescriptions[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
}

const std::string Response::toString( void ) const
{
	std::ostringstream ss;
    ss << getMajorVersion();
	std::string major = ss.str();
	ss << getMinorVersion();
	std::string minor = ss.str();
	std::string contentType = getMimeType(this->_request.getResource());

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
	if (this->_headerSection.size() > 0)
	{
		r += "Content-Length: " + contentLength + CRLF;
		r += this->_headerSection;
	}
	else
	{
		r += "Content-Type: " + contentType + CRLF;
		r += "Content-Length: " + contentLength + CRLF;
		r += CRLF; // Empty line between headers and content
	}

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

std::string Response::readStaticPage( void ) const
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

void Response::clearEnv( char **env )
{
	for (int i = 0; i < MAX_ENV; i++)
		delete [] env[i];
	delete [] env;
}

char **Response::getEnv( void )
{
	std::string base = this->_connection.gets("dynamic_route");
	std::string path = this->_request.getResource();
	std::string method = "";
	
	if (this->_request.getMethod() == GET)
		method = "GET";
	else if (this->_request.getMethod() == POST)
		method = "POST";
	else if (this->_request.getMethod() == DELETE) {
		method = "DELETE";

		std::string path = this->_connection.gets("static_route") + this->_request.getResource();
		if (std::remove(path.c_str()) == 0) {
			this->_status = OK;
		} else {
			this->_status = NOT_FOUND;
		}
	}
	else
	{
		this->_status = NOT_IMPLEMENTED;
		return NULL;
	}

	char **env = new char*[MAX_ENV];

	if (env != NULL)
	{
		std::string e = "PATH_INFO=" + path;
		env[0] = new char[e.size() + 1];
		if (env[0])
			env[0] = strcpy(env[0], e.c_str());
	
		e = "SCRIPT_NAME=index.php";
		env[1] = new char[e.size() + 1];
		if (env[1])
			env[1] = strcpy(env[1], e.c_str());
	
		e = "PATH_TRANSLATED=" + base + path;
		env[2] = new char[e.size() + 1];
		if (env[2])
			env[2] = strcpy(env[2], e.c_str());
	
		e = "GATEWAY_INTERFACE=CGI/1.1";
		env[3] = new char[e.size() + 1];
		if (env[3])
			env[3] = strcpy(env[3], e.c_str());
	
		e = "REQUEST_METHOD=" + method;
		env[4] = new char[e.size() + 1];
		if (env[4])
			env[4] = strcpy(env[4], e.c_str());
	
		e = "REDIRECT_STATUS=200";
		env[5] = new char[e.size() + 1];
		if (env[5])
			env[5] = strcpy(env[5], e.c_str());
	
		e = "SCRIPT_FILENAME=" + base + path;
		env[6] = new char[e.size() + 1];
		if (env[6])
			env[6] = strcpy(env[6], e.c_str());
	
		e = "SERVER_PROTOCOL=HTTP/1.1";
		env[7] = new char[e.size() + 1];
		if (env[7])
			env[7] = strcpy(env[7], e.c_str());
	
		e = "SERVER_PORT=80";
		env[8] = new char[e.size() + 1];
		if (env[8])
			env[8] = strcpy(env[8], e.c_str());
	
		e = "REQUEST_URI=/";
		env[9] = new char[e.size() + 1];
		if (env[9])
			env[9] = strcpy(env[9], e.c_str());
	
		e = "SERVER_SOFTWARE=zweb/1.1";
		env[10] = new char[e.size() + 1];
		if (env[10])
			env[10] = strcpy(env[10], e.c_str());

		e = "CONTENT_TYPE=" + this->_request.header("Content-Type");
		env[11] = new char[e.size() + 1];
		if (env[11])
			env[11] = strcpy(env[11], e.c_str());

		e = "CONTENT_LENGTH=" + this->_request.header("Content-Length");
		env[12] = new char[e.size() + 1];
		if (env[12])
			env[12] = strcpy(env[12], e.c_str());
	
		e = "REMOTE_HOST=" + this->_request.header("Host");
		env[13] = new char[e.size() + 1];
		if (env[13])
			env[13] = strcpy(env[13], e.c_str());

		e = "QUERY_STRING=" + this->_request.getQueryString();
		env[14] = new char[e.size() + 1];
		if (env[14])
			env[14] = strcpy(env[14], e.c_str());

		env[15] = NULL;
	}

	return env;
}

std::string Response::readDynamicPage( void )
{
	int stdin = dup(STDIN_FILENO);
	int stdout = dup(STDOUT_FILENO);
	int	fd[2];
	std::string response = "";

	if (pipe(fd) == -1)
	{
		this->_status = INTERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] creating pipe");
		return "";
	}

	char **env = getEnv();
	if (env == NULL)
	{
		close(fd[0]);
		close(fd[1]);
		return "";
	}

	pid_t pid = fork();
	if (pid < 0)
	{
		this->_status = INTERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] not abled to create fork");
		clearEnv(env);
		return "";
	}
	else if (!pid)
	{
		char **cmd = new char*[2];
		cmd[0] = (char *) "";
		cmd[1] = NULL;
		dup2(this->_clientSocket, STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execve(CGI_PHP, cmd, env);
		this->_status = INTERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] error executing CGI");
		delete [] cmd;
	}
	else
	{
		char buffer[CGI_BUFFSIZE] = {0};
		waitpid(-1, NULL, 0);
		int j = 1;
		while (j > 0)
		{
			memset(buffer, 0, CGI_BUFFSIZE);
			j = read(fd[0], buffer, CGI_BUFFSIZE - 1);
			response += buffer;
			if (j < CGI_BUFFSIZE)
				break;
		}
		close(fd[0]);
		close(fd[1]);
	}

	dup2(stdin, STDIN_FILENO);
	dup2(stdout, STDOUT_FILENO);

	close(stdin);
	close(stdout);

	clearEnv(env);

	if (pid == 0)
		exit(0);

	return getParsedCGIResponse(response);
}

std::string const Response::getParsedCGIResponse( std::string const response )
{
	int counter = 0;
	bool compile = false;
	std::string parsed = "";
	for (size_t i = 0; i < response.size(); i++)
	{
		if (!compile)
			this->_headerSection += response[i];
		if (response[i] == 10 || response[i] == 13)
		{
			counter++;
			if (counter == 4)
				compile = true;
			continue;
		}
		else
			counter = 0;
		if (compile)
			parsed += response[i];
	}
	return (parsed);
}

std::string Response::getMimeType(const std::string& path) const {
	std::string extension = "";

	if (path.compare("/") == 0)
		return "text/html";
	size_t pos = path.find_last_of('.');
	if (pos == std::string::npos) {
		return "application/octet-stream"; // No extension found
	}
	
	extension = path.substr(pos + 1);

	for (size_t i = 0; i < extension.length(); i++) {
		extension[i] = std::tolower(extension[i]);
	}

	if (extension == "html" || extension == "html") return "text/html";
	else if (extension == "css") return "text/css";
	else if (extension == "js") return "application/javascript";
	else if (extension == "json") return "application/json";
	else if (extension == "jpg" || extension == "jpeg") return "image/jpeg";
	else if (extension == "png") return "image/png";
	else if (extension == "gif") return "image/gif";
	else if (extension == "svg") return "image/svg+xml";
	else if (extension == "ico") return "image/x-icon";
	else if (extension == "pdf") return "application/pdf";
	else if (extension == "zip") return "applcation/zip";
	else if (extension == "xml") return "application/xml";
	else if (extension == "txt") return "text/plain";
	else if (extension == "mp4") return "video/mp4";
	else if (extension == "mp3") return "audio/mpeg";
	else if (extension == "wav") return "audio/wav";
	else if (extension == "php") return "text/html";

	return "application/octet-stream"; // Default MIME type for unknown extensions
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
