#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>
# include "Message.hpp"
# include "Connection.hpp"

class Connection;

typedef enum { GET, POST, PUT, DELETE, PATCH, OPTIONS, TRACE, HEAD, UNKNOWN } Method;

class Request : public Message
{

	public:

		Request(Method method, std::string resource, int major, int minor, Connection & connection);
		~Request();

		Request & operator=( Request const & rhs );

		Method getMethod( void ) const;
		std::string getResource( void ) const;

		const std::string getMethodString( void ) const;
		std::string const header( std::string const header ) const;

		std::string const getQueryString( void ) const;

	private:
		Method _method;
		std::string _resource;
		std::string _queryString;
		std::string _body;
		Connection & _connection;
		std::map<std::string, std::string> _headers;

};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
