#ifndef CONTEXT_HPP
# define CONTEXT_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>
# include "Context.hpp"

class Context
{
	public:
		Context( );
		virtual ~Context();
		Context & operator=( Context const & rhs );

		std::string getRoot( void ) const;
		void setRoot( std::string root );
		std::pair<int, std::string> getErrorPage( void ) const;
		void setErrorPage( int statusCode, std::string page );
		std::pair<int, std::string> getReturn( void ) const;
		void setReturn( int statusCode, std::string page );
		std::vector<std::string> getIndex( void ) const;
		void setIndex( std::string index );
		bool getAutoIndex( void ) const;
		void setAutoIndex( bool autoIndex );

		virtual int getPort( void ) const;
		virtual void setPort( int port );
		virtual std::vector<std::string> getServerNames( void ) const;
		virtual void setServerName( std::string serverName );

		virtual std::string getPath( void ) const;
		virtual void setPath( std::string path );

	protected:
		std::string _root;
		std::pair<int, std::string> _errorPage;
		std::pair<int, std::string> _return;
		std::vector<std::string> _index;
		bool _autoIndex;
		
		// NOT WIRED
		long _clientMaxBodySize;
};

std::ostream & operator<<( std::ostream & o, Context const & i );

#endif
