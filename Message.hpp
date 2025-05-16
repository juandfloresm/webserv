#ifndef MESSAGE_HPP
# define MESSAGE_HPP

# include <iostream>
# include <string>
# include <map>

# include "Configuration.hpp"

class Message
{

	public:
		virtual ~Message();

		int getMajorVersion( void ) const;
		int getMinorVersion( void ) const;
		void setMajorVersion( int major );
		void setMinorVersion( int minor );

	protected:
		Message(int socket, Configuration & cfg);

		int _majorVersion;
		int _minorVersion;

		Configuration & _cfg;
		int _clientSocket;

};

#endif
