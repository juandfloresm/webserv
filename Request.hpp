#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include <map>
# include "Message.hpp"

typedef enum { GET, POST, PUT, DELETE, PATCH, OPTIONS, TRACE, HEAD, UNKNOWN } Method;

class Request : public Message
{

	public:

		Request(Method method, std::string resource, std::string raw, int major, int minor);
		~Request();

		Request & operator=( Request const & rhs );

		Method getMethod( void ) const;
		std::string getResource( void ) const;

		const std::string getMethodString( void ) const;
		std::string const header( std::string const header ) const;

	private:
		Method _method;
		std::string _resource;
		std::string _body;
		std::string _raw;
		std::map<std::string, std::string> _headers;

};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
