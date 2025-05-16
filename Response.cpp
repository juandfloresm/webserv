#include "Response.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Response::Response(Status status, int clientSocket, Configuration & cfg, int port, Request & request) : Message(clientSocket, cfg), _status(status), _port(port), _request(request)
{
	setMajorVersion(MAJOR_VERSION);
	setMinorVersion(MINOR_VERSION);
	initStatusDescriptions();
	this->_headerSection = "";
	this->_page = "";
	if (status == OK)
	{
		try {
			matchServer();
			matchLocation();
			doResponse();
		} catch ( Response::NotFoundException & e ) {
			errorHandler(NOT_FOUND);
		} catch ( Response::ForbiddenException & e ) {
			errorHandler(FORBIDDEN);
		} catch ( Response::ContentTooLargeException & e ) {
			errorHandler(CONTENT_TOO_LARGE);
		} catch ( Response::InternalServerException & e ) {
			errorHandler(INTERNAL_SERVER_ERROR);
		} catch ( Response::NotImplementedException & e ) {
			errorHandler(NOT_IMPLEMENTED);
		} catch ( Response::BadGatewayException & e ) {
			errorHandler(BAD_GATEWAY);
		}
	}
	else
		doSend(clientSocket);
}


/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Response::~Response() {}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/


/*
** --------------------------------- METHODS ----------------------------------
*/

void Response::matchServer( void )
{
	ServerList list = _cfg.getServerList();
	ServerList match = _cfg.getServerList();
	ServerList::iterator it = list.begin();

	for(; it < list.end(); it++) // ................................... by PORT
		if (it->getPort() == this->_port)
			match.push_back(*it);

	if (match.size() == 0)
		this->_server = match.back();
	else if (match.size() > 0)
	{
		for(it = match.begin(); it < match.end(); it++) // ..... by SERVER_NAME
		{
			std::vector<std::string> list = it->getServerNames();
			std::vector<std::string>::iterator sit = list.begin();
			for(; sit < list.end(); sit++)
			{
				if ((*sit).compare(this->_request.header("Host")) == 0)
				{
					this->_server = *it;
					return ;
				}
			}
		}
		this->_server = match.at(0); // ..................... by DEFAULT_SERVER
	}

	throw Response::BadGatewayException();
}

void Response::matchLocation( void )
{
	std::string requestPath = _request.getResource();
	std::vector<Location> lcs = this->_server.getLocations();
	Location loc;
	bool matched = false;
	if (lcs.size() > 0)
	{
		for (std::vector<Location>::iterator lit = lcs.begin(); lit < lcs.end(); lit++)
		{
			if (matchLocationExact(lit->getPath(), requestPath))
			{
				loc = *lit;
				matched = true;
				_prefix = loc.getPath();
				_request.setResource("/");
				break ;
			}
		}
		if (!matched)
		{
			int i = 0, t;
			for (std::vector<Location>::iterator lit = lcs.begin(); lit < lcs.end(); lit++)
			{
				t = matchLocationLogestPrefix(lit->getPath(), requestPath);
				if (t > i)
				{
					i = t;
					loc = *lit;
					matched = true;
				}
			}
			if (matched)
			{
				_prefix = loc.getPath();
				std::string r = _request.getResource();
				_request.setResource(requestPath.substr(loc.getPath().size()));
			}
		}
		if (!matched)
			throw NotFoundException();	
		else
		{
			if (loc.getRoot().compare("./html") != 0)
				this->_server.setRoot(loc.getRoot());
			this->_server.setAutoIndex(loc.getAutoIndex());
			this->_location = loc;
		}
	}
}

bool Response::matchLocationExact( std::string locationPath, std::string requestPath )
{	
	if (locationPath.compare(".php") == 0 && requestPath.rfind(".php") != std::string::npos)
		return true;

	if (*requestPath.rbegin() == '/' && *locationPath.rbegin() != '/')
		locationPath += "/";

	if(locationPath.find(requestPath) != std::string::npos && locationPath.size() == requestPath.size())
		return true;

	return false;
}

int Response::matchLocationLogestPrefix( std::string locationPath, std::string requestPath )
{
	int i = 0;
	std::istringstream f(locationPath);
	std::istringstream r(requestPath);
	std::string locationSegment, requestSegment;
	while (getline(f, locationSegment, '/') && getline(r, requestSegment, '/') && locationSegment.compare(requestSegment) == 0)
		i++;
	return i;
}

void Response::doResponse( void )
{
	int code = this->_server.getReturn().first;
	if (this->_server.getAutoIndex() && isDirectory())
	{
		this->_content = readDirectory();
		doSend(_clientSocket);
	}
	else if(code != 0)
	{
		std::string page = this->_server.getReturn().second;
		if (code < static_cast<int>(BAD_REQUEST))
			redirectCode(code, page);
		else
			throwErrorCode(code, page);
	}
	else if (this->_location.getPassCGI().size() > 0)
	{
		pid_t pid = fork();
		if (pid < 0)
			throw InternalServerException();
		else if (pid == 0)
		{
			this->_content = readDynamicPage();
			doSend(_clientSocket);
			exit(0);
		}
	}
	else
	{
		this->_content = readStaticPage();
		doSend(_clientSocket);
	}
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
	close(_clientSocket);
}

const std::string Response::toString( void ) const
{
	std::ostringstream ss;
    ss << getMajorVersion();
	std::string major = ss.str();
	ss.str("");
	ss.clear();
	ss << getMinorVersion();
	std::string minor = ss.str();
	std::string contentType = getMimeType(this->_request.getResource());

	if (isDirectory() && this->_server.getAutoIndex())
		contentType = "text/html";

	ss.str("");
	ss.clear();
	ss << this->_contentLength;
	std::string contentLength = ss.str();

	std::string r = "";

	r += "HTTP/" + major + "." + minor + " ";
	
	if (this->_headerSection.size() > 0)
	{
		std::size_t start = this->_headerSection.find("Status:");
		if (start != std::string::npos)
		{
			std::size_t end = this->_headerSection.find(CRLF, start);
			std::string top = this->_headerSection.substr(start + 7, end);
			r += top + CRLF;
		}
		else
			r += this->_statusString + " " + getDescription() + CRLF;
		r += "Content-Length: " + contentLength + CRLF;
		r += this->_headerSection;
	}
	else
	{
		r += this->_statusString + " " + getDescription() + CRLF;
		r += "Content-Type: " + contentType + CRLF;
		r += "Content-Length: " + contentLength + CRLF;
		if (this->_page.size() > 0)
			r += "Location: " + this->_page + CRLF;
		r += CRLF;
	}

	r += this->_content;

	return r;
}

std::string Response::readError( std::string filePath ) const
{
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		ft_error("No error file match " + filePath);
		return "";
	}
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

std::string Response::readDirectory( void ) const
{
	std::string base = this->_server.getRoot();
	std::string path = this->_request.getResource();
	std::string filePath = base + path;
	std::string result = "<h1>Directory: " + filePath + "</h1><ul>";
	struct dirent* file;
	DIR* fd;

	std::vector<std::string> dirs;
	std::vector<std::string> files;

	fd = opendir(filePath.c_str());
	if (fd == NULL)
		throw ForbiddenException();

	if (chdir(filePath.c_str()) != 0)
		throw ForbiddenException();

	while ((file = readdir(fd)))
	{
		struct stat buffer;
		int status;
		status = stat(file->d_name, &buffer);
		if (status == -1)
			throw ForbiddenException();
		if ( buffer.st_mode & S_IFREG )
			files.push_back(file->d_name);
		else
			dirs.push_back(file->d_name);
	}

 	closedir(fd);

	path = _prefix + path;
	if (*path.rbegin() != '/')
		path += "/";

	std::sort(dirs.begin(), dirs.end());
	for (std::vector<std::string>::iterator it = dirs.begin(); it < dirs.end(); it++)
		result += ("<li>&#128193; <a href=\"" + (path + *it) + "\">" + *it + "</a></li>");
	std::sort(files.begin(), files.end());
	for (std::vector<std::string>::iterator it = files.begin(); it < files.end(); it++)
		result += ("<li>&#128462; <a href=\"" + (path + *it) + "\">" + *it + "</a></li>");

	result += "</ul>";
	return result;
}

std::string Response::readStaticPage( void ) const
{
	std::string base = this->_server.getRoot();
	std::string path = this->_request.getResource();

	if (path.empty() || path[0] != '/')
		path = "/" + path;
	
	std::string filePath = base + path;
	
	if (isDirectory())
	{
		std::vector<std::string> list = this->_server.getIndex();
		std::vector<std::string>::iterator sit = list.begin();
		bool found = false;
		for(; sit < list.end(); sit++)
		{
			if (access((filePath + *sit).c_str(), F_OK) == 0)
			{
				filePath += *sit;
				found = true;
				break;
			}
		}
		if(!found)
		{
			filePath = (filePath + "/" + DEFAULT_PAGE);
			this->_request.setResource(path + "/" + DEFAULT_PAGE);
		}
	}

	std::cout << "[STATIC] Serving file: " << filePath << std::endl;
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open())
		throw NotFoundException();
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	return content.str();
}

std::string Response::readDynamicPage( void )
{
	std::string binary = _location.getPassCGI();
	std::cout << "[DYNAMIC] Serving file: " << _request.getResource() << std::endl;
	int stdin = dup(STDIN_FILENO);
	int stdout = dup(STDOUT_FILENO);
	int	fd[2];
	std::string response = "";

	if (pipe(fd) == -1)
	{
		this->_status = INTERNAL_SERVER_ERROR;
		ft_error("Creating pipe");
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
		ft_error("Not abled to create fork");
		clearEnv(env);
		return "";
	}
	else if (!pid)
	{
		char **cmd = new char*[2];
		cmd[0] = (char *) "";
		cmd[1] = NULL;
		dup2(_clientSocket, STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execve(binary.c_str(), cmd, env);
		this->_status = INTERNAL_SERVER_ERROR;
		ft_error("Executing CGI");
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

/*
** --------------------------------- UTILITIES ---------------------------------
*/

void Response::errorHandler( Status status )
{
	ft_error(this->_statusDescriptions[status]);
	this->_status = status;
	setErrorPage(status);
	doSend(_clientSocket);
}


void Response::ft_error( const std::string err ) const
{
	if (errno != 0)
		perror("");
	std::cerr << "[Error] " << (err + ".........................................CONTEXT = '" + _request.getResource() + "'") << std::endl;
}

void Response::setErrorPage(int status)
{
	std::vector<std::pair<int, std::string> > ep = _server.getErrorPages();
	for (std::vector<std::pair<int, std::string> >::iterator it = ep.begin(); it < ep.end(); it++)
	{
		if (it->first == static_cast<int>(status))
		{
			this->_content = readError(it->second);
			break ;
		}
	}
}

bool Response::isDirectory( void ) const
{
	std::string path = this->_request.getResource();
	return path.find(".") == std::string::npos;
}

void Response::redirectCode( int code, std::string page )
{
	this->_status = static_cast<Status>(code);
	this->_page = page;
	doSend(_clientSocket);
}

void Response::throwErrorCode( int code, std::string page )
{
	if (page.size() > 0)
		this->_content = readError(page);
	else
		setErrorPage(code);
	this->_status = static_cast<Status>(code);
	doSend(_clientSocket);
}

void Response::clearEnv( char **env )
{
	int i = 0;
	while (env[i])
		delete [] env[i++];
	delete [] env;
}

void Response::setSingleEnv(char **env, std::string const s, int i)
{
	env[i] = new char[s.size() + 1];
	if (env[i])
		env[i] = strcpy(env[i], s.c_str());	
}

char **Response::getEnv( void )
{
	std::string base = this->_server.getRoot();
	std::string path = this->_request.getResource();
	std::string method = "";
	
	if (this->_request.getMethod() == GET)
		method = "GET";
	else if (this->_request.getMethod() == POST)
		method = "POST";
	else if (this->_request.getMethod() == DELETE) {
		method = "DELETE";
		std::string path = base + this->_request.getResource();
		if (std::remove(path.c_str()) == 0)
			this->_status = OK;
		else
			this->_status = NOT_FOUND;
	}
	else
		throw NotImplementedException();

	Header headers = this->_request.getHeaders();
	std::vector<std::string> headerList;
	HeaderIterator it = headers.begin();
	std::string key;
	for (; it != headers.end(); it++)
	{
		key = headerTransform(it->first);
		headerList.push_back("HTTP_" + key + "=" + it->second);
	}
	headerList.push_back("SCRIPT_NAME=" + base + path);
	headerList.push_back("SCRIPT_FILENAME=" + base + path);
	headerList.push_back("PATH_INFO=" + base + path);
	headerList.push_back("PATH_TRANSLATED=" + base + path);
	headerList.push_back("REQUEST_URI=/");
	headerList.push_back("QUERY_STRING=" + this->_request.getQueryString());
	headerList.push_back("GATEWAY_INTERFACE=CGI/1.1");
	headerList.push_back("REQUEST_METHOD=" + method);
	headerList.push_back("REDIRECT_STATUS=200");
	headerList.push_back("SERVER_PROTOCOL=HTTP/1.1");
	headerList.push_back("SERVER_PORT=80");
	headerList.push_back("SERVER_SOFTWARE=zweb/1.1");
	headerList.push_back("REMOTE_HOST=" + this->_request.header("Host"));

	std::string auth = this->_request.header("Authorization");
	if (auth.length() > 0)
	{
		std::string scheme, userId;
		std::istringstream f(auth);
		getline(f, scheme, ' ');
		getline(f, userId, ' ');
		headerList.push_back("AUTH_TYPE=" + scheme);
		headerList.push_back("REMOTE_USER=" + userId);
	}

	char **env = new char*[headerList.size() + 1];
	if (env != NULL)
	{
		for (size_t i = 0; i < headerList.size(); i++)
			setSingleEnv(env, headerList[i], i);
		env[headerList.size()] = NULL;
	}

	return env;
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
	this->_statusDescriptions[CONTENT_TOO_LARGE] = "Content Too Large";
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

std::string Response::headerTransform(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(), Response::headerCharTransform);
    return s;
}

unsigned char Response::headerCharTransform(unsigned char c)
{
	if (c == '-')
		c = '_';
	return std::toupper(c); 
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
