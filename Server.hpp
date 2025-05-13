#ifndef SERVER_HPP
# define SERVER_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include "Context.hpp"
# include "Location.hpp"

class Server : public Context
{
	public:
		Server( );
		~Server();
		Server & operator=( Server const & rhs );

		int getPort( void ) const;
		void setPort( int port );
		std::vector<std::string> getServerNames( void ) const;
		void setServerName( std::string serverName );
		std::vector<Location> getLocations( void ) const;
		void setLocation( Location location );

	private:
		int _port;
		std::vector<std::string> _serverNames;
		std::vector<Location> _locations;
};

std::ostream & operator<<( std::ostream & o, Server const & i );

#endif
