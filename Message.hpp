#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <iostream>
# include <string>
# include <map>

typedef std::map<std::string, std::string> Header;
typedef Header::iterator HeaderIterator;

class Message
{

	public:
		virtual ~Message();

		int getMajorVersion( void ) const;
		int getMinorVersion( void ) const;

	protected:
		Message(int major, int minor);
		Message & operator=( Message const & rhs );

		int _majorVersion;
		int _minorVersion;

		Header _headers;

};

std::ostream & operator<<( std::ostream & o, Message const & i );

#endif
