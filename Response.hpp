#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <string>
# include <map>
# include <sstream>
# include <algorithm>
# include <fstream>

# include <csignal>
# include <cstring>
# include <sys/wait.h>
# include <dirent.h>
# include <errno.h>
# include <stdio.h>
# include <sys/stat.h>

# include "Message.hpp"
# include "Request.hpp"
# include "Server.hpp"

# define CRLF "\r\n"
# define CGI_BUFFSIZE 2048
# define MAJOR_VERSION 1
# define MINOR_VERSION 1

# define DEFAULT_PAGE "index.html"

typedef std::map<std::string, std::string> Config;

typedef enum {

	CONTINUE = 100,
	SWITCHING_PROTOCOLOS = 101,

	/* 200 - 299 .............................. */
	OK = 200,
	CREATED = 201,
	ACCEPTED = 202,
	NO_CONTENT = 204,
	PARTIAL_CONTENT = 206,

	/* 300 - 399 .............................. */
	MULTIPLE_CHOICES = 300,
	MOVED_PERMANENTLY = 301,
	FOUND = 302,
	SEE_OTHER = 303,
	NOT_MODIFIED = 304,
	TEMPORARY_REDIRECT = 307,
	PERMANENET_REDIRECT = 308,

	/* 400 - 499 .............................. */
	BAD_REQUEST = 400,
	UNAUTHORIZED = 401,
	FORBIDDEN = 403,
	NOT_FOUND = 404,
	METHOD_NOT_ALLOWED = 405,
	NOT_ACCEPTABLE = 406,
	REQUEST_TIMEOUT = 408,
	CONFLICT = 409,
	GONE = 410,
	LENGTH_REQUIRED = 411,
	CONTENT_TOO_LARGE = 413,
	URI_TOO_LONG = 414,
	UNSUPPORTED_MEDIA_TYPE = 415,
	EXPECTATION_FAILED = 426,
	TOO_MANY_REQUESTS = 429,


	/* 500 - 599 .............................. */
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT = 504,
	HTTP_VERSION_NOT_SUPPORTED = 505

} Status;

typedef std::map<Status, std::string> StatusDescription;
typedef StatusDescription::iterator StatusDescriptionIterator;

class Request;

class Response : public Message
{

	public:

		Response(Status status, int clientSocket, Configuration & cfg, int port, Request & request);
		~Response();

		Response & operator=( Response const & rhs );

		Status getStatus( void ) const;
		std::string getDescription( void ) const;

		void initStatusDescriptions( void );
		const std::string toString( void ) const;

		std::string readError( std::string status ) const;
		std::string readStaticPage( void ) const;
		std::string readDynamicPage( void );
		void clearEnv( char **env );
		char **getEnv( void );

		void doResponse( void );
		void doSend( int fd );

		std::string const getParsedCGIResponse( std::string const response );
		void setSingleEnv(char **env, std::string const s, int i);
		std::string headerTransform(std::string s);
		static unsigned char headerCharTransform(unsigned char c);
		void matchServer( void );

		void redirectCode( int code, std::string page );
		void throwErrorCode( int code, std::string page );
		std::string readDirectory( void ) const;
		bool isDirectory( void ) const;

		void errorHandler( Status status );
		void matchLocation( void );
		void setErrorPage(int status);
		bool matchLocationExact( std::string locationPath, std::string requestPath );
		int matchLocationLogestPrefix( std::string locationPath, std::string requestPath );
		void ft_error( const std::string err ) const;

		/* 400 */
		class NotFoundException : public std::exception {
			public:
				const char * what () { return "Not Found"; }
		};
		/* 403 */
		class ForbiddenException : public std::exception {
			public:
				const char * what () { return "Forbidden"; }
		};
		/* 413 */
		class ContentTooLargeException : public std::exception {
			public:
				const char * what () { return "Content Too Large"; }
		};
		/* 500 */
		class InternalServerException : public std::exception {
			public:
				const char * what () { return "Internal Server Error"; }
		};
		/* 501 */
		class NotImplementedException : public std::exception {
			public:
				const char * what () { return "Not Implemented"; }
		};
		/* 502 */
		class BadGatewayException : public std::exception {
			public:
				const char * what () { return "Bad Gateway"; }
		};

	private:
		std::string _statusString;
		std::string _description;
		std::string _headerSection;
		std::string _page;
		std::string _content;
		std::string _prefix;
		Status _status;
		StatusDescription _statusDescriptions;
		long _contentLength;
		int _port;
		Request & _request;
		Server _server;
		Location _location;

		std::string getMimeType(const std::string& path) const;
};

std::ostream & operator<<( std::ostream & o, Response const & i );

#endif
