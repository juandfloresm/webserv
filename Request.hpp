#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>
# include "Message.hpp"
# include "Connection.hpp"

# define BUFFER 1
# define LF 10
# define CR 13

class Connection;

typedef enum { GET, POST, PUT, DELETE, PATCH, OPTIONS, TRACE, HEAD, UNKNOWN } Method;

typedef std::map<std::string, std::string> Header;
typedef Header::iterator HeaderIterator;

class Request : public Message
{

	public:

		Request(int clientSocket);
		~Request();

		Request & operator=( Request const & rhs );

		Method getMethod( void ) const;
		std::string getResource( void ) const;

		const std::string getMethodString( void ) const;
		std::string header( std::string const header );

		std::string const getQueryString( void ) const;

		void parseRequest( void );
		std::string getMessageLine( void );
		void parseHeaders( void );
		Header & getHeaders( void );

	private:
		Method _method;
		std::string _resource;
		std::string _queryString;
		std::string _body;
		int & _clientSocket;
		Header _headers;
		char _buffer[BUFFER];
		static const char HEADER_SEP;
};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
