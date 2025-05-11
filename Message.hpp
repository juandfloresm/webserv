#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <iostream>
# include <string>
# include <map>

class Message
{

	public:
		virtual ~Message();

		int getMajorVersion( void ) const;
		int getMinorVersion( void ) const;
		void setMajorVersion( int major );
		void setMinorVersion( int minor );

	protected:
		Message();
		Message(int major, int minor);
		Message & operator=( Message const & rhs );

		int _majorVersion;
		int _minorVersion;

};

std::ostream & operator<<( std::ostream & o, Message const & i );

#endif
