#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration( std::string const configFile ) 
{
	level0.push_back("server");
	level1.push_back("listen");
	level1.push_back("root");
	level1.push_back("server_name");
	level1.push_back("index");
	level1.push_back("error_page");
	level1.push_back("return");
	level1.push_back("autoindex");
	level1.push_back("location");
	level2.push_back("methods");
	level2.push_back("cgi_pass");
	level2.push_back("root");
	level2.push_back("index");
	level2.push_back("autoindex");
	level2.push_back("error_page");
	level2.push_back("return");

	levels[0] = level0;
	levels[1] = level1;
	levels[2] = level2;
	parse(configFile);
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Configuration::~Configuration(){}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/

Configuration & Configuration::operator=( Configuration const & rhs )
{
	(void) rhs;
	return *this;
}

std::ostream & operator<<( std::ostream & o, Configuration const & i )
{
	(void) i;
	o << "Configuration = " << std::endl;
	return o;
}


/*
** --------------------------------- METHODS ----------------------------------
*/

bool Configuration::isSpace(char c)
{
	return (c==' ' || c=='\n' || c=='\t' || c=='\v' || c=='\r' || c=='\f');
}

bool Configuration::isEnding(char c, int * i)
{
	if (c=='{' || c=='}')
	{
		std::cout << *i << " to ";
		if (c == '{')
			(*i)++;
		else if (c == '}')
			(*i)--;
		std::cout << *i << std::endl;
		return true;
	}
	return false;
}

void Configuration::parse( std::string const file )
{
	int fd = open(file.c_str(), O_RDONLY);
	if (fd < 0)
		throw std::exception();
	else 
	{
		char c;
		int i = 0;
		std::string token = "";
		while (read(fd, &c, 1) > 0)
		{
			if (isSpace(c))
			{
				processToken( fd, token, &i );
				token = "";
			}
			else
			{
				token.push_back(c);
			}
		}
		if (token.size() > 0)
			processToken( fd, token, &i );
		if(i != 0)
		{
			std::cerr << "[Error] parsing format error: " << i << std::endl;
			throw std::exception();
		}
	}
}

void Configuration::processToken( int fd, std::string token, int * i )
{
	char c;
	if(!isEnding(token[0], i))
	{
		if (isTokenValid(*i, token))
		{
			std::string value = "";
			while (read(fd, &c, 1) > 0)
			{
				if (c == ';' || ((token.compare("server") == 0 || token.compare("location") == 0) && isEnding(c, i)))
					break;
				else if (isEnding(c, i) || c == '\n')
				{
					std::cerr << "[Error] Error processing token: [" << token << "]. No end in site" << std::endl;
					throw std::exception();
				}
				value.push_back(c);
			}
			parseEntry(std::make_pair(token, value));
		}
		else if(token.size() > 0)
		{
			std::cerr << "[Error] Invalid token: [" << token << "]" << std::endl;
			throw std::exception();
		}
	}
}


bool Configuration::isTokenValid(int i, std::string token)
{
	for (size_t j = 0; j < levels[i].size(); j++)
	{
		if (token.compare(levels[i][j]) == 0)
			return true;
	}
	return false;
}

void Configuration::parseEntry( Entry directive )
{
	if (directive.first.compare("server") == 0)
	{
		this->_servers.push_back(Server());
		this->_parsingServer = true;
	}
	else if (directive.first.compare("location") == 0)
	{
		this->_location = Location();
		std::istringstream f(directive.second);
		std::string s;
		getline(f, s, ' ');
		this->_location.setPath(s);
		this->_servers.back().setLocation(this->_location);
		this->_parsingServer = false;
	}
	else if (this->_parsingServer)
		parseContext(this->_servers.back(), directive);
	else
		parseContext(this->_servers.back().getLastLocation(), directive);
}

void Configuration::parseContext( Context & cxt, Entry directive )
{
	if (directive.first.compare("listen") == 0)
		cxt.setPort(port(directive.second));
	else if (directive.first.compare("root") == 0)
		cxt.setRoot(word(directive.second));
	else if (directive.first.compare("server_name") == 0)
	{
		std::istringstream f(directive.second);
		std::string s;
		while (getline(f, s, ' '))
			cxt.setServerName(word(s));
	}
	else if (directive.first.compare("error_page") == 0)
	{
		std::istringstream f(directive.second);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		cxt.setErrorPage(statusCode(code), word(page));
	}
	else if (directive.first.compare("return") == 0)
	{
		std::istringstream f(directive.second);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		cxt.setReturn(statusCode(code), word(page));
	}
	else if (directive.first.compare("index") == 0)
	{
		std::istringstream f(directive.second);
		std::string s;
		cxt.getIndex().clear();
		while (getline(f, s, ' '))
			cxt.setIndex(word(s));
	}
	else if (directive.first.compare("autoindex") == 0)
	{
		if (flag(directive.second).compare("on") == 0)
			cxt.setAutoIndex(true);
		else
			cxt.setAutoIndex(false);
	}
	else if (directive.first.compare("methods") == 0)
	{
		std::istringstream f(directive.second);
		std::string s;
		cxt.getMethods().clear();
		while (getline(f, s, ' '))
			cxt.setMethod(methods(word(s)));
	}
	else if (directive.first.compare("cgi_pass") == 0)
		cxt.setPassCGI(word(directive.second));
}

int Configuration::port( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	size_t n = 0, i = 1;
	getline(f, s, ' ');
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 5)
		{
			std::cerr << "[Error] port cannot be greater than range [0 - 65.535]" << std::endl;
			throw std::exception();
		}
		if (!isdigit(*it))
		{
			std::cerr << "[Error] provided directive value should be a number '" << *it << "'" << std::endl;
			throw std::exception();
		}
		i++;
	}
	n = std::atoi(s.c_str());
	if (n < 1024 || n > 65535)
	{
		std::cerr << "[Error] port cannot be greater than range [1024 - 65.535]" << std::endl;
		if (n < 1024)
		{
			std::cerr << n << " is a well-known port for which admins credentials might be needed." << std::endl;
		}
		throw std::exception();
	}
	return n;
}

std::string Configuration::word( std::string raw )
{
	std::istringstream f(raw);
	std::string s, extra = "/.";
	size_t i = 0;
	getline(f, s, ' ');
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 256)
		{
			std::cerr << "[Error] excessive number of token characters" << std::endl;
			throw std::exception();
		}
		if (isalpha(*it) || (extra.find(*it) != std::string::npos) || (i > 0 && isdigit(*it)))
		{
			// good
		}
		else
		{
			std::cerr << "[Error] provided directive value should be a word: '" << raw << "'" << std::endl;
			throw std::exception();
		}
		i++;
	}
	return s;
}

std::string Configuration::methods( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	getline(f, s, ' ');
	if (s.compare("GET") == 0 || s.compare("POST") == 0 || s.compare("DELETE") == 0)
		return s;
	else
	{
		std::cerr << "[Error] unsupported method '" << raw << "'" << std::endl;
		throw std::exception();
	}
}

std::string Configuration::flag( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	getline(f, s, ' ');
	if (s.compare("on") == 0 || s.compare("off") == 0)
		return s;
	else
	{
		std::cerr << "[Error] unsupported flag value '" << raw << "'" << std::endl;
		throw std::exception();
	}
}

int Configuration::statusCode( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	size_t n = 0, i = 1;
	getline(f, s, ' ');
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 3)
		{
			std::cerr << "[Error] status code cannot be greater than range [100 - 599]" << std::endl;
			throw std::exception();
		}
		if (!isdigit(*it))
		{
			std::cerr << "[Error] provided directive value is should be a number" << std::endl;
			throw std::exception();
		}
		i++;
	}
	n = std::atoi(s.c_str());
	if (n < 100 || n > 599)
	{
		std::cerr << "[Error] port cannot be greater than range [100 - 599]" << std::endl;
		throw std::exception();
	}
	return n;
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
ServerList & Configuration::getServerList( void )
{
	return this->_servers;
}
