#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <string>
# include "Message.hpp"

typedef enum { OK, REDIRECT, NOT_FOUND, SERVER_ERROR } Status;

class Response : public Message
{

	public:

		Response(Status status, std::string description, int major, int minor);
		~Response();

		Response & operator=( Response const & rhs );

		Status getStatus( void ) const;
		std::string getDescription( void ) const;

	private:
		Status _status;
		std::string _description;
};

std::ostream & operator<<( std::ostream & o, Response const & i );

#endif
