#ifndef CONTEXT_HPP
# define CONTEXT_HPP

# include <iostream>
# include <string>
# include <map>
# include <vector>

class Context
{
	public:
		Context( );
		Context( const Context & c );
		virtual ~Context();
		Context & operator=( Context const & rhs );

		std::string getRoot( void ) const;
		void setRoot( std::string root );
		std::vector<std::pair<int, std::string> > getErrorPages( void ) const;
		void setErrorPage( int statusCode, std::string page );
		std::pair<int, std::string> getReturn( void ) const;
		void setReturn( int statusCode, std::string page );
		std::vector<std::string> getIndex( void ) const;
		void setIndex( std::string index );
		std::vector<std::string> getMimeTypes( void ) const;
		void setMimeType( std::string type );
		void setMimeTypes( std::vector<std::string> types );
		bool getAutoIndex( void ) const;
		void setAutoIndex( bool autoIndex );
		unsigned long getClientMaxBodySize( void ) const;
		void setClientMaxBodySize( unsigned long sz );
		std::string getAuthBasic( void ) const;
		void setAuthBasic( std::string authBasic );
		std::string getUploadPath( void ) const;
		void setUploadPath( std::string path );

		virtual int getPort( void ) const;
		virtual void setPort( int port );

		virtual std::string getPath( void ) const;
		virtual void setPath( std::string path );
		virtual std::vector<std::string> getMethods( void ) const;
		virtual void setMethod( std::string method );
		virtual std::string getPassCGI( void ) const;
		virtual void setPassCGI( std::string passGCI );
		virtual std::string getHost() const;
		virtual void setHost( std::string host );

	protected:
		std::string _root;
		std::vector<std::pair<int, std::string> > _errorPages;
		std::pair<int, std::string> _return;
		std::vector<std::string> _index;
		std::vector<std::string> _mimeTypes;
		std::string _authBasic;
		std::string _uploadPath;
		bool _autoIndex;
		unsigned long _clientMaxBodySize;
};

#endif
