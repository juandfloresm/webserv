#ifndef REQUEST_HPP
# define REQUEST_HPP

# include <iostream>
# include <string>
# include "Message.hpp"

typedef enum { GET, POST, PUT, DELETE, PATCH, OPTIONS, TRACE } Method;

class Request : public Message
{

	public:

		Request(Method method, std::string resource, int major, int minor);
		~Request();

		Request & operator=( Request const & rhs );

		Method getMethod( void ) const;
		std::string getResource( void ) const;

	private:
		Method _method;
		std::string _resource;

};

std::ostream & operator<<( std::ostream & o, Request const & i );

#endif
