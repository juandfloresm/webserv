#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>

# include <sys/socket.h>

# include "Message.hpp"
# include "Response.hpp"

# define BUFFER 1
# define LF 10
# define CR 13

typedef enum { GET, POST, DELETE, HEAD, PUT, CONNECT, OPTIONS, TRACE, PATCH, UNKNOWN } Method;

typedef std::map<std::string, std::string> Header;
typedef Header::iterator HeaderIterator;

class Request : public Message
{

	public:

		Request(int clientSocket, Configuration & cfg, int port);
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

};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
