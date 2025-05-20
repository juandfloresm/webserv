#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <fstream>
# include <cstdlib>
# include <cstdio>
# include <ctime>
# include <map>

# include <sys/socket.h>

# include "Message.hpp"
# include "Response.hpp"

# define BUFFER 1
# define LF 10
# define CR 13
# define CRLFF "\r\f"

# define CONTENT_TYPE "Content-Type"
# define CONTENT_LENGTH "Content-Length"
# define FORM_TYPE_MULTIPART "multipart/form-data"
# define FORM_TYPE_APPLICATION "application/x-www-form-urlencoded"
# define FORM_TYPE_PLAIN "text/plain"
# define CONTENT_DISPOSITION "Content-Disposition: form-data; name=\""
# define TRANSFER_ENCODING "Transfer-Encoding"
# define CHUNKED "chunked"
# define SESSION_KEY "ZWEBSESSID"
# define BASE64_HASH "Basic YWRtaW46MTIzNA=="
# define BASE_16 "0123456789abcdefABCDEF"
# define UPLOAD_PATH "/html/uploads/"
# define ZWEB_FILENAME "ZWEB_FILENAME_"

#define SSTR( x ) static_cast< std::ostringstream & >( \
        ( std::ostringstream() << std::dec << x ) ).str()

typedef enum { GET, POST, DELETE, HEAD, PUT, CONNECT, OPTIONS, TRACE, PATCH, UNKNOWN } Method;

typedef std::map<std::string, std::string> Header;
typedef Header::iterator HeaderIterator;
typedef std::map<std::string, std::string> Session;
typedef std::map<std::string, Session> Sessions;

class Request : public Message
{

	public:

		Request(int clientSocket, Configuration & cfg, int port, Sessions & sess);
		~Request();

		Method getMethod( void ) const;
		std::string getResource( void ) const;
		void setResource( std::string resource );

		const std::string getMethodString( void ) const;
		std::string header( std::string const header );

		std::string const getQueryString( void ) const;

		void parseTopLine( void );
		void parseHeaders( void );
		void parseContent( unsigned long clientMaxBodySize );

		std::string getMessageLine( void );
		Header & getHeaders( void );
		bool isContentAvailable( void ) const;
		bool isFormContentType( void );
		void parseContentLength( void );
		void parseContentType( void );
		bool eq( std::string s1, std::string s2 );
		void parseMultipartContent( void );
		void parseContentPart( void );
		void setPart(std::string & name, std::string & value);
		std::vector<std::string> split(std::string & s, std::string& delimiter, bool last);
		void parseContentFragment( unsigned long max, unsigned long n );
		void parseChunkedContent( unsigned long clientMaxBodySize );
		void p( std::string s ) const;
		Session getSession( void );
		bool isInSession( void );
		std::string getSessionCookie( void );
		std::string getSessionId( void ) const;
		void fdBody( void );
		std::string getBody( void ) const;
		int getBodyFD( void );
		std::string randomString(const int len);
		void removeBodyFD( void );
		std::string processFileUpload( void );
		std::string getFilePath( void ) const;
		void headerDelegate( std::string key, std::string value );

	private:
		std::string _resource;
		std::string _queryString;
		std::string _body;
		std::string _contentType;
		std::string _boundary;
		std::string _charSet;
		unsigned long _contentLength;
		Method _method;
		Header _headers;
		char _buffer[BUFFER];
		static const char HEADER_SEP;

		std::map<std::string, std::string> _content;
		Sessions & _sessions;
		Session _session;
		std::string _sessionId;
		int _bodyFD;
		std::string _fdFile;
};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
