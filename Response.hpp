#ifndef RESPONSE_HPP
# define RESPONSE_HPP

# include <iostream>
# include <string>
# include <map>
# include "Message.hpp"
# include "Connection.hpp"

# define CRLF "\r\n"

class Connection;

typedef enum { 

	/* 200 - 299 .............................. */
	OK = 200,

	/* 300 - 399 .............................. */
	MOVED_PERMANENTLY = 301, 

	/* 400 - 499 .............................. */
	NOT_FOUND = 404, 

	/* 500 - 599 .............................. */
	INSERNAL_SERVER_ERROR = 500

} Status;

typedef std::map<Status, std::string> StatusDescription;
typedef StatusDescription::iterator StatusDescriptionIterator;

class Response : public Message
{

	public:

		Response(Status status, int major, int minor, const Connection & connection);
		~Response();

		Response & operator=( Response const & rhs );

		Status getStatus( void ) const;
		std::string getDescription( void ) const;

		void initStatusDescriptions( void );
		const std::string toString( void ) const;

		std::string readError( std::string status ) const;

		void sampleResonseSetup( void );
		void send( void );

	private:
		Status _status;
		std::string _statusString;
		std::string _description;
		StatusDescription _statusDescriptions;
		const Connection & _connection;
		std::string _content;
		long _contentLength;
};

std::ostream & operator<<( std::ostream & o, Response const & i );

#endif
