#include "Response.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Response::Response(Status status, int major, int minor, const Connection & connection, const Request & request) : Message(major, minor), _status(status), _connection(connection), _request(request)
{
	initStatusDescriptions();
	sampleStaticResponse();
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

void Response::sampleStaticResponse( void )
{
	std::ostringstream ss;
    ss << this->_status;
	this->_content = readStaticPage();
	this->_contentLength = this->_content.size();
	this->_description = this->_statusDescriptions[this->_status];
	this->_statusString = ss.str();
	doSend(this->_connection.getClientSocket());
}

void Response::sampleDynamicResponse( void )
{
	std::ostringstream ss;
    ss << this->_status;
	this->_content = readDynamicPage();
	this->_contentLength = this->_content.size();
	this->_description = this->_statusDescriptions[this->_status];
	this->_statusString = ss.str();
	doSend(this->_connection.getClientSocket());
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

std::string Response::readDynamicPage( void )
{
	int stdin = dup(STDIN_FILENO);
	int stdout = dup(STDOUT_FILENO);
	int	fd[2];
	char **cmd = NULL;
	std::string response;

	if (pipe(fd) == -1)
	{
		this->_status = INSERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] creating pipe");
		return "";
	}



	char **env = new char*[MAX_ENV];
	std::string e = "PATH_INFO=./cgi-bin";
	env[0] = new char[e.size() + 1];
	env[0] = strcpy(env[0], e.c_str());

	e = "SCRIPT_NAME=test.php";
	env[1] = new char[e.size() + 1];
	env[1] = strcpy(env[1], e.c_str());

	e = "PATH_TRANSLATED=./cgi-bin/test.php";
	env[2] = new char[e.size() + 1];
	env[2] = strcpy(env[2], e.c_str());

	env[3] = NULL;



	pid_t pid = fork();
	if (pid < 0)
	{
		this->_status = INSERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] not abled to create fork");
		clearEnv(env);
		return "";
	}
	else if (!pid)
	{
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		execve(CGI_PHP, cmd, env);
		this->_status = INSERNAL_SERVER_ERROR;
		this->_connection.ft_error("[Error] error executing CGI");
		clearEnv(env);
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

	return response;
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
