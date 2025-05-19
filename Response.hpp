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
# define DEFAULT_ERROR_PAGE "./config/ERROR"
# define LAST_DATE "Sun, 28 May 2025 7:00:00 GMT" // TODO: Sunday after release
# define SESSION_ESTABLISHED "<center style=\"color:blue\">SESSION ESTABLISHED!!</center>"

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

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
	EXPECTATION_FAILED = 417,
	UNPROCESSABLE_CONTENT = 422,
	UPGRADE_REQUIRED = 426,
	TOO_MANY_REQUESTS = 429,


	/* 500 - 599 .............................. */
	INTERNAL_SERVER_ERROR = 500,
	NOT_IMPLEMENTED = 501,
	BAD_GATEWAY = 502,
	SERVICE_UNAVAILABLE = 503,
	GATEWAY_TIMEOUT = 504,
	HTTP_VERSION_NOT_SUPPORTED = 505

} Status;

typedef std::map<std::string, std::string> Config;
typedef std::map<Status, std::string> StatusDescription;
typedef StatusDescription::iterator StatusDescriptionIterator;


class Request;

class Response : public Message
{

	public:

		Response(Status status, int clientSocket, Configuration & cfg, int port, Request & request);
		~Response();

		Status getStatus( void ) const;
		std::string getDescription( void ) const;

		void initStatusDescriptions( void );
		std::string toString( void );

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
		void setErrorPage( void );
		bool matchLocationExact( std::string locationPath, std::string requestPath );
		int matchLocationLogestPrefix( std::string locationPath, std::string requestPath );
		void showError( const std::string err ) const;
		void validateLocationMethods( void ) const;
		std::string replaceAll( std::string str, std::string from, std::string to ) const;
		void p( std::string s ) const;
		void checkAuthorization( void );

		bool processUpload( void );
		bool processDelete( void );
		bool isContentAvailable( void ) const;
		bool isPOST( void ) const;
		bool isDELETE( void ) const;
		bool isCGI( void ) const;
		bool eq( std::string s1, std::string s2 ) const;
		std::string getFilePath( void ) const;

		/* 400 */
		class BadRequestException : public std::exception {
			public:
				const char * what () { return "Bad Request"; }
		};
		/* 401 */
		class UnauthorizedException : public std::exception {
			public:
				const char * what () { return "Unauthorized"; }
		};
		/* 403 */
		class ForbiddenException : public std::exception {
			public:
				const char * what () { return "Forbidden"; }
		};
		/* 404 */
		class NotFoundException : public std::exception {
			public:
				const char * what () { return "Not Found"; }
		};
		/* 405 */
		class MethodNotAllowedException : public std::exception {
			public:
				const char * what () { return "Method Not Allowed"; }
		};
		/* 411 */
		class LengthRequiredException : public std::exception {
			public:
				const char * what () { return "Length Required"; }
		};
		/* 413 */
		class ContentTooLargeException : public std::exception {
			public:
				const char * what () { return "Content Too Large"; }
		};
		/* 415 */
		class UnsupportedMediaTypeException : public std::exception {
			public:
				const char * what () { return "Unsupported Media Type"; }
		};
		/* 422 */
		class UnprocessableContentException : public std::exception {
			public:
				const char * what () { return "Unprocessable Content"; }
		};
		/* 426 */
		class UpgradeRequiredException : public std::exception {
			public:
				const char * what () { return "Upgrade Required"; }
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
		std::string _contentType;
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

#endif
