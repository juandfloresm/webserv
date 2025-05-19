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
		Server( const Server & server );
		~Server();
		Server & operator=( Server const & rhs );

		int getPort( void ) const;
		void setPort( int port );
		std::string getHost() const;
		void setHost( std::string host );
		std::vector<Location> getLocations( void ) const;
		void setLocation( Location location );
		bool isDefault( void ) const;
		void setDefault( bool _default );
		Location & getLastLocation( void );

	private:
		std::string _host;
		int _port;
		bool _default;
		std::vector<Location> _locations;
};

#endif
