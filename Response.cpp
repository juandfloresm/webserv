#include "Response.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Response::Response(Status status, int clientSocket, Configuration & cfg, int port, Request & request) : Message(clientSocket, cfg), _status(status), _port(port), _request(request)
{
	setMajorVersion(MAJOR_VERSION);
	setMinorVersion(MINOR_VERSION);
	initStatusDescriptions();
	_headerSection = "";
	_page = "";
	if (status == OK)
	{
		try {
			matchServer();
			matchLocation();
			doResponse();
		} catch ( Response::ForbiddenException & e ) {
			errorHandler(FORBIDDEN);
		} catch ( Response::UnauthorizedException & e ) {
			errorHandler(UNAUTHORIZED);
		} catch ( Response::BadRequestException & e ) {
			errorHandler(BAD_REQUEST);
		} catch ( Response::NotFoundException & e ) {
			errorHandler(NOT_FOUND);
		} catch ( Response::MethodNotAllowedException & e ) {
			errorHandler(METHOD_NOT_ALLOWED);
		} catch ( Response::ContentTooLargeException & e ) {
			errorHandler(CONTENT_TOO_LARGE);
		} catch ( Response::InternalServerException & e ) {
			errorHandler(INTERNAL_SERVER_ERROR);
		} catch ( Response::ImATeaPotException & e ) {
			errorHandler(TEAPOT);
		} catch ( Response::NotImplementedException & e ) {
			errorHandler(NOT_IMPLEMENTED);
		} catch ( Response::BadGatewayException & e ) {
			errorHandler(BAD_GATEWAY);
		}
	}
	else
		errorHandler(status);
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
	ServerList::iterator it = list.begin();
	std::string host = _request.header("Host");
	for(; it < list.end(); it++) // ................................... by HOST
	{
		if (eq(it->getHost(), host))
		{
			_server = *it;
			return;
		}
	}
	it = list.begin();
	for(; it < list.end(); it++) // ................................... by PORT
	{
		if (it->getPort() == _port)
		{
			_server = *it;
			return;
		}
	}
	throw Response::BadGatewayException();
}

void Response::matchLocation( void )
{
	std::string requestPath = _request.getResource();
	std::vector<Location> lcs = _server.getLocations();
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
		}
		if (!matched)
			throw NotFoundException();	
		else
		{
			if (!eq(loc.getRoot(), "./html"))
				_server.setRoot(loc.getRoot());
			if (loc.getClientMaxBodySize() > 0)
				_server.setClientMaxBodySize(loc.getClientMaxBodySize());
			if (loc.getClientMaxHeaderSize() > 0)
				_server.setClientMaxHeaderSize(loc.getClientMaxHeaderSize());
			if (!loc.getAuthBasic().empty())
				_server.setAuthBasic(loc.getAuthBasic());
			if (loc.getMimeTypes().size() > 0)
				_server.setMimeTypes(loc.getMimeTypes());
			if (!loc.getUploadPath().empty())
				_server.setUploadPath(loc.getUploadPath());
			if (loc.getReturn().first > 0)
				_server.setReturn(loc.getReturn().first, loc.getReturn().second);
			_server.setAutoIndex(loc.getAutoIndex());
			_location = loc;
			p(requestPath + ", " + "Mached: " + loc.getPath() + ", Interpreting: " + _request.getResource());
		}
	}
}

bool Response::matchLocationExact( std::string locationPath, std::string requestPath )
{	
	if (eq(locationPath, ".php") && requestPath.rfind(".php") != std::string::npos)
		return true;
	if (eq(locationPath, ".py") && requestPath.rfind(".py") != std::string::npos)
		return true;
	if (eq(locationPath, ".pl") && requestPath.rfind(".pl") != std::string::npos)
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
	while (getline(f, locationSegment, '/') && getline(r, requestSegment, '/') && eq(locationSegment, requestSegment))
		i++;
	return i;
}

void Response::validateLocationMethods( void ) const
{
	std::vector<std::string> methods = _location.getMethods();
	const std::string method = _request.getMethodString();
	if (methods.size() > 0)
	{
		std::vector<std::string>::iterator it = methods.begin();
		for( ; it < methods.end(); it++)
		{
			if (eq(method, *it))
				return ;
		}
		throw MethodNotAllowedException();
	}
}

void Response::checkAuthorization( void )
{
	if (!_server.getAuthBasic().empty() && !_request.isInSession() && _request.getSessionId().empty())
		throw UnauthorizedException();
}

void Response::checkHeaderSize( void )
{
	if (_server.getClientMaxHeaderSize() > 0 && _server.getClientMaxHeaderSize() < _request.getHeaderLength())
		throw URITooLongException();
}

void Response::doResponse( void )
{
	_cfg.setServer(_server);										// ................................... SHARE SERVER/LOCATION

	validateLocationMethods();										// ................................... CHECK MATCHING METHODS

	checkAuthorization();											// ................................... CHECK AUTHORIZED REQUESTS

	checkHeaderSize();												// ................................... CHECK HEADER SIZE

	_request.parseContent(_server.getClientMaxBodySize());			// ................................... CHECK CONTENT SIZE

	if (isPOST() && !isCGI() && isContentAvailable())				// ................................... FILE UPLOAD
	{
		if (processUpload())
			return;
	}	
	if (isDELETE() && !isCGI())										// ................................... DELETE FILE
	{
		if (processDelete())
			return;
	}
	
	if(_server.getReturn().first != 0)								// ................................... RETURN
	{
		int code = _server.getReturn().first;
		std::string page = _server.getReturn().second;
		if (code < static_cast<int>(BAD_REQUEST))
			redirectCode(code, page);
		else
			throwErrorCode(code, page);
	}
	else if (_server.getAutoIndex() && isDirectory())				// ................................... AUTO INDEX
	{
		_content = readDirectory();
		doSend(_clientSocket);
	}
	else if (isCGI())												// ................................... DYNAMIC
	{
		_content = readDynamicPage();
		doSend(_clientSocket);
	}
	else															// ................................... STATIC
	{
		_content = readStaticPage();
		doSend(_clientSocket);
	}
}

void Response::doSend( int fd )
{
	std::ostringstream ss;
	ss << _status;
	_description = _statusDescriptions[_status];
	_statusString = ss.str();
	std::string resp = toString();
	send(fd, resp.c_str(), resp.size(), 0);
	close(_clientSocket);
}

std::string Response::toString( void )
{
	std::ostringstream ss;
    ss << getMajorVersion();
	std::string major = ss.str();
	ss.str("");
	ss.clear();
	ss << getMinorVersion();
	std::string minor = ss.str();
	std::string contentType = _contentType;
	
	if (contentType.empty())
	{
		contentType = getMimeType(_request.getResource());

		if (isDirectory() && _server.getAutoIndex())
			contentType = "text/html";
	}

	ss.str("");
	ss.clear();

	if (_request.isInSession())
		_content += SESSION_ESTABLISHED;

	_contentLength = _content.size();

	ss << _contentLength;
	std::string contentLength = ss.str();

	std::string r = "";

	r += "HTTP/" + major + "." + minor + " ";
	
	if (_headerSection.size() > 0)
	{
		std::size_t start = _headerSection.find("Status:");
		if (start != std::string::npos)
		{
			std::size_t end = _headerSection.find(CRLF, start);
			std::string top = _headerSection.substr(start + 7, end);
			r += top + CRLF;
		}
		else
		{
			r += _statusString + " " + getDescription() + CRLF;
		}
		r += "Content-Length: " + contentLength + CRLF;
		r += _headerSection;
	}
	else
	{
		r += _statusString + " " + getDescription() + CRLF;

		if(!_request.getSessionId().empty())
		{
			std::string key = SESSION_KEY;
			r += "Set-Cookie: " + key + "=" + _request.getSessionId() + CRLF;
		}
		else if (_status == UNAUTHORIZED)
		{
			std::string realm = _server.getAuthBasic();
			std::string date = LAST_DATE;
			r += "Date: " + date + CRLF;
			r += "WWW-Authenticate: Basic realm=\"" + realm + "\"" + CRLF;
			r += CRLF;
			return r;
		}

		r += "Content-Type: " + contentType + CRLF;
		r += "Content-Length: " + contentLength + CRLF;
		if (_page.size() > 0)
			r += "Location: " + _page + CRLF;
		r += CRLF;
	}

	r += _content;

	return r;
}

std::string Response::replaceAll( std::string str, std::string from, std::string to ) const
{
	size_t pos = str.find(from);
	while (pos != std::string::npos)
	{
		str.replace(pos, from.length(), to);
		pos = str.find(from, pos + to.length());
	}
	return str;
}

std::string Response::readError( std::string filePath )
{
	std::ifstream file(filePath.c_str(), std::ios::binary);
	if (!file.is_open()) {
		showError("No error file match " + filePath);
		return "";
	}
	std::ostringstream content;
	content << file.rdbuf();
	file.close();
	std::string desc = _statusDescriptions.at(_status);
	std::string s = content.str();
	_contentType = "text/html";
	return replaceAll(replaceAll(s, "[STATUS_CODE]", SSTR(_status)), "[STATUS_DESCRIPTION]", desc);
}

std::string Response::readDirectory( void ) const
{
	std::string base = _server.getRoot();
	std::string path = _request.getResource();
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

std::string Response::getFilePath( void ) const
{
	std::string base = _server.getRoot();
	std::string path = _request.getResource();

	if (path.empty() || path[0] != '/')
		path = "/" + path;
	
	std::string filePath = base + path;
	return filePath;
}

std::string Response::readStaticPage( void ) const
{
	std::string path = _request.getResource();
	std::string filePath = getFilePath();
	
	if (isDirectory())
	{
		std::vector<std::string> list = _server.getIndex();
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
			_request.setResource(path + "/" + DEFAULT_PAGE);
		}
	}

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
	std::string script = _server.getRoot() + _request.getResource();

	int stdin = dup(STDIN_FILENO);
	int stdout = dup(STDOUT_FILENO);
	int	fd[2];
	std::string response = "";

	if (pipe(fd) == -1)
	{
		_status = INTERNAL_SERVER_ERROR;
		showError("Creating pipe");
		return "";
	}

	char **env = getEnv();
	if (env == NULL)
	{
		close(fd[0]);
		close(fd[1]);
		return "";
	}

	_request.fdBody();
	pid_t pid = fork();
	if (pid < 0)
	{
		_status = INTERNAL_SERVER_ERROR;
		showError("Not abled to create fork");
		clearEnv(env);
		return "";
	}
	else if (!pid)
	{
		char **cmd = new char*[3];
		cmd[0] = (char *) binary.c_str();
		cmd[1] = (char *) script.c_str();
		cmd[2] = NULL;
		int bfd = _request.getBodyFD();
		dup2(bfd, STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(bfd);
		close(fd[0]);
		close(fd[1]);
		execve(cmd[0], cmd, env);
		_status = INTERNAL_SERVER_ERROR;
		showError("Executing CGI");
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
		_request.removeBodyFD();
	}

	dup2(stdin, STDIN_FILENO);
	dup2(stdout, STDOUT_FILENO);

	close(stdin);
	close(stdout);

	clearEnv(env);

	if (pid == 0)
		std::exit(0);

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
			_headerSection += response[i];
		if (response[i] == LF || response[i] == CR)
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

void Response::p( std::string s ) const
{
	std::cout << s << std::endl;
}

bool Response::eq( std::string s1, std::string s2 ) const
{
	return (s1.compare(s2) == 0 && s1.size() == s2.size());
}

void Response::errorHandler( Status status )
{
	showError(_statusDescriptions[status]);
	_status = status;
	setErrorPage();
	doSend(_clientSocket);
}

void Response::showError( const std::string err ) const
{
	strerror(errno);
	std::cerr << "[Error] " << (err + ".........................................CONTEXT = '" + _request.getResource() + "'") << std::endl;
}

void Response::setErrorPage(void)
{
	std::vector<std::pair<int, std::string> > ep = _server.getErrorPages();
	for (std::vector<std::pair<int, std::string> >::iterator it = ep.begin(); it < ep.end(); it++)
	{
		if (it->first == static_cast<int>(_status))
		{
			_content = readError(it->second);
			break ;
		}
	}
	_content = readError(DEFAULT_ERROR_PAGE);
}

bool Response::isDirectory( void ) const
{
	std::string path = _request.getResource();
	return path.find(".") == std::string::npos;
}

void Response::redirectCode( int code, std::string page )
{
	_status = static_cast<Status>(code);
	_page = page;
	doSend(_clientSocket);
}

void Response::throwErrorCode( int code, std::string page )
{
	_status = static_cast<Status>(code);
	if (page.size() > 0)
		_content = readError(page);
	else
		setErrorPage();
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
		env[i] = std::strcpy(env[i], s.c_str());	
}

char **Response::getEnv( void )
{
	std::string base = _server.getRoot();
	std::string path = _request.getResource();
	std::string method = _request.getMethodString();

	Header headers = _request.getHeaders();
	std::vector<std::string> headerList;
	HeaderIterator it = headers.begin();
	std::string key;
	for (; it != headers.end(); it++)
	{
		key = headerTransform(it->first);
		headerList.push_back("HTTP_" + key + "=" + it->second);
	}
	headerList.push_back("PATH_INFO=" + base + path);
	headerList.push_back("SCRIPT_NAME=" + base + path);
	headerList.push_back("SCRIPT_FILENAME=" + base + path);
	headerList.push_back("PATH_TRANSLATED=" + base + path);
	headerList.push_back("REQUEST_URI=/");
	headerList.push_back("QUERY_STRING=" + _request.getQueryString());
	headerList.push_back("GATEWAY_INTERFACE=CGI/1.1");
	headerList.push_back("REQUEST_METHOD=" + method);
	headerList.push_back("REDIRECT_STATUS=200");
	headerList.push_back("SERVER_PROTOCOL=HTTP/1.1");
	headerList.push_back("SERVER_PORT=80");
	headerList.push_back("SERVER_SOFTWARE=zweb/1.1");
	headerList.push_back("REMOTE_HOST=" + _request.header("Host"));
	headerList.push_back("CONTENT_TYPE=" + _request.header("Content-Type"));
	headerList.push_back(SSTR("CONTENT_LENGTH=" << _request.getBody().size()));

	std::string auth = _request.header("Authorization");
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
	_statusDescriptions[CONTINUE] = "Continue";
	_statusDescriptions[SWITCHING_PROTOCOLOS] = "Switching Protocols";

	/* 200 - 299 .............................. */
	_statusDescriptions[OK] = "OK";
	_statusDescriptions[CREATED] = "Created";
	_statusDescriptions[ACCEPTED] = "Accepted";
	_statusDescriptions[NO_CONTENT] = "No Content";
	_statusDescriptions[RESET_CONTENT] = "Reset Content";
	_statusDescriptions[PARTIAL_CONTENT] = "Partial Content";

	/* 300 - 399 .............................. */
	_statusDescriptions[MULTIPLE_CHOICES] = "Multiple Choices";
	_statusDescriptions[MOVED_PERMANENTLY] = "Moved Permanently";
	_statusDescriptions[FOUND] = "Found";
	_statusDescriptions[SEE_OTHER] = "See Other";
	_statusDescriptions[NOT_MODIFIED] = "Not Modified";
	_statusDescriptions[TEMPORARY_REDIRECT] = "Temporary Redirect";
	_statusDescriptions[PERMANENET_REDIRECT] = "Permanent Redirect";

	/* 400 - 499 .............................. */
	_statusDescriptions[BAD_REQUEST] = "Bad Request";
	_statusDescriptions[UNAUTHORIZED] = "Unauthorized";
	_statusDescriptions[FORBIDDEN] = "Forbidden";
	_statusDescriptions[NOT_FOUND] = "Not Found";
	_statusDescriptions[METHOD_NOT_ALLOWED] = "Method Not Allowed";
	_statusDescriptions[NOT_ACCEPTABLE] = "Not Acceptable";
	_statusDescriptions[REQUEST_TIMEOUT] = "Request Timeout";
	_statusDescriptions[CONFLICT] = "Conlifct";
	_statusDescriptions[GONE] = "Gone";
	_statusDescriptions[LENGTH_REQUIRED] = "Length Required";
	_statusDescriptions[PRECONDITION_FAILED] = "Precondition Failed";
	_statusDescriptions[CONTENT_TOO_LARGE] = "Content Too Large";
	_statusDescriptions[URI_TOO_LONG] = "URI Too Long";
	_statusDescriptions[UNSUPPORTED_MEDIA_TYPE] = "Unsupported Media Type";
	_statusDescriptions[RANGE_NOT_SATISFIABLE] = "Range Not Satisfiable";
	_statusDescriptions[UNPROCESSABLE_CONTENT] = "Unprocessable Content";
	_statusDescriptions[UPGRADE_REQUIRED] = "Upgrade Required";
	_statusDescriptions[EXPECTATION_FAILED] = "Expectation Failed";
	_statusDescriptions[TEAPOT] = "I'm a teapot";
	_statusDescriptions[TOO_MANY_REQUESTS] = "Too Many Requests";

	/* 500 - 599 .............................. */
	_statusDescriptions[INTERNAL_SERVER_ERROR] = "Internal Server Error";
	_statusDescriptions[NOT_IMPLEMENTED] = "Not Implemented";
	_statusDescriptions[BAD_GATEWAY] = "Bad Gateway";
	_statusDescriptions[SERVICE_UNAVAILABLE] = "Service Unavailable";
	_statusDescriptions[GATEWAY_TIMEOUT] = "Gateway Timeout";
	_statusDescriptions[HTTP_VERSION_NOT_SUPPORTED] = "HTTP Version Not Supported";
}

std::string Response::getMimeType(const std::string& path) const {
	std::string extension = "";

	if (eq(path, "/"))
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

bool Response::isCGI( void ) const
{
	return _location.getPassCGI().size() > 0;
}

bool Response::isPOST( void ) const
{
	return eq(_request.getMethodString(), "POST");
}

bool Response::isDELETE( void ) const
{
	return eq(_request.getMethodString(), "DELETE");
}

bool Response::isContentAvailable( void ) const
{
	return _request.isContentAvailable() && _request.isFormContentType();
}

bool Response::processUpload( void )
{
	p("Processing file upload...");
	std::string path = _request.processFileUpload();
	if (!path.empty())
	{
		_page = path;
		_status = CREATED;
		_content = _request.getBody();
		_contentType = FORM_TYPE_PLAIN;
		doSend(_clientSocket);
		return true;
	}
	else
		_status = RESET_CONTENT;
	return false;
}

bool Response::processDelete( void )
{
	p("Processing file delete...");
	std::string filePath = getFilePath();
	if(std::remove(filePath.c_str()) == 0)
	{
		_status = NO_CONTENT;
		_content = "";
		doSend(_clientSocket);
	}
	else
	{
		if (access(filePath.c_str(), F_OK) == 0)
			throw MethodNotAllowedException();
		else
			throw NotFoundException();
	}
	return true;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/

Status Response::getStatus( void ) const
{
	return _status;
}

std::string Response::getDescription( void ) const
{
	return _description;
}
