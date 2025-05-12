#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>

class Server
{

	public:
		Server();
		~Server();
		Server & operator=( Server const & rhs );

		int getPort( void ) const;
		void setPort( int port );

	private:
		int _port;

};

std::ostream & operator<<( std::ostream & o, Server const & i );

#endif
