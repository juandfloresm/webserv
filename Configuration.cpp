#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration( std::string const configFile ) 
{
	_directiveCount = -1;

	level0.push_back("server");
	level1.push_back("listen");
	level1.push_back("root");
	level1.push_back("index");
	level1.push_back("error_page");
	level1.push_back("return");
	level1.push_back("autoindex");
	level1.push_back("location");
	level1.push_back("client_max_body_size");
	level1.push_back("client_max_header_size");
	level1.push_back("auth_basic");
	level1.push_back("mime_types");
	level1.push_back("upload_path");
	level2.push_back("methods");
	level2.push_back("cgi_pass");
	level2.push_back("root");
	level2.push_back("index");
	level2.push_back("autoindex");
	level2.push_back("error_page");
	level2.push_back("return");
	level2.push_back("client_max_body_size");
	level2.push_back("client_max_header_size");
	level2.push_back("auth_basic");
	level2.push_back("mime_types");
	level2.push_back("upload_path");

	levels[0] = level0;
	levels[1] = level1;
	levels[2] = level2;

	_wpath = std::getenv("WPATH");

	parse(configFile);
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Configuration::~Configuration(){}


/*
** --------------------------------- OVERLOAD ---------------------------------
*/


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
		if (c == '{')
			(*i)++;
		else if (c == '}')
			(*i)--;
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
			throw std::runtime_error(SSTR("[Error] parsing format error: " << i));
		if (_directiveCount == 0)
			throw std::runtime_error("[Error] no directives detected for context");
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
				if (c == ';' || ((eq(token, "server") || eq(token, "location")) && isEnding(c, i)))
					break;
				else if (isEnding(c, i) || c == '\n')
					throw std::runtime_error(SSTR("[Error] Error processing token: [" << token << "]. No end in site"));
				value.push_back(c);
			}
			parseEntry(std::make_pair(token, value));
		}
		else if(token.size() > 0)
			throw std::runtime_error(SSTR("[Error] Invalid token: [" << token << "]"));
	}
}


bool Configuration::isTokenValid(int i, std::string token)
{
	for (size_t j = 0; j < levels[i].size(); j++)
	{
		if (eq(token, levels[i][j]))
			return true;
	}
	return false;
}

void Configuration::parseEntry( Entry directive )
{
	if (eq(directive.first, "server"))
	{
		if (_directiveCount == 0)
			throw std::runtime_error("[Error] no directives detected for context");
		this->_servers.push_back(Server());
		this->_parsingServer = true;
		_directiveCount = 0;
	}
	else if (eq(directive.first, "location"))
	{
		if (_directiveCount == 0)
			throw std::runtime_error("[Error] no directives detected for context");
		this->_location = Location();
		this->_location.setPath(path(directive.second));
		this->_servers.back().setLocation(this->_location);
		this->_parsingServer = false;
		_directiveCount = 0;
	}
	else if (this->_parsingServer)
		parseContext(this->_servers.back(), directive);
	else
		parseContext(this->_servers.back().getLastLocation(), directive);
}

void Configuration::parseContext( Context & cxt, Entry directive )
{
	std::string parsedValue = directive.second;
	std::string var = "$[WPATH]";
	if (parsedValue.find(var) != std::string::npos)
		parsedValue = parsedValue.replace(parsedValue.find(var), var.size(), _wpath);
	
	_directiveCount++;

	if (eq(directive.first, "listen"))
	{
		if (parsedValue.find(" ") != std::string::npos)
			throw std::runtime_error("Bad listen directive format");
		else if (parsedValue.find(":") == std::string::npos)
			cxt.setPort(port(parsedValue));
		else
		{
			std::istringstream f(parsedValue);
			std::string shost, sport;
			getline(f, shost, ':');
			getline(f, sport, ':');
			cxt.setPort(port(sport));
			if (eq(sport, "80"))
				cxt.setHost(shost);
			else
				cxt.setHost(parsedValue);
		}
	}
	else if (eq(directive.first, "root"))
		cxt.setRoot(path(parsedValue));
	else if (eq(directive.first, "error_page"))
	{
		std::istringstream f(parsedValue);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		cxt.setErrorPage(statusCode(code), path(page));
	}
	else if (eq(directive.first, "return"))
	{
		std::istringstream f(parsedValue);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		if (!page.empty())
			page = path(page);
		cxt.setReturn(statusCode(code), page);
	}
	else if (eq(directive.first, "index"))
	{
		std::istringstream f(parsedValue);
		std::string s;
		cxt.getIndex().clear();
		while (getline(f, s, ' '))
			cxt.setIndex(word(s));
	}
	else if (eq(directive.first, "autoindex"))
	{
		if (eq(flag(parsedValue), "on"))
			cxt.setAutoIndex(true);
		else
			cxt.setAutoIndex(false);
	}
	else if (eq(directive.first, "methods"))
	{
		std::istringstream f(parsedValue);
		std::string s;
		cxt.getMethods().clear();
		while (getline(f, s, ' '))
			cxt.setMethod(methods(word(s)));
	}
	else if (eq(directive.first, "cgi_pass"))
		cxt.setPassCGI(path(parsedValue));
	else if (eq(directive.first, "client_max_body_size"))
		cxt.setClientMaxBodySize(size(parsedValue));
	else if (eq(directive.first, "client_max_header_size"))
		cxt.setClientMaxHeaderSize(size(parsedValue));
	else if (eq(directive.first, "auth_basic"))
		cxt.setAuthBasic(word(parsedValue));
	else if (eq(directive.first, "mime_types"))
	{
		std::istringstream f(parsedValue);
		std::string s;
		cxt.getMimeTypes().clear();
		while (getline(f, s, ' '))
			cxt.setMimeType(word(s));
	}
	else if (eq(directive.first, "upload_path"))
		cxt.setUploadPath(path(parsedValue));
}

/*
** --------------------------------- VALIDATIONS ------------------------------------------------------------------
*/

int Configuration::port( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	size_t n = 0, i = 1;
	getline(f, s, ' ');
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 5)
			throw std::runtime_error("[Error] port cannot be greater than range [0 - 65.535]");
		if (!std::isdigit(*it))
			throw std::runtime_error(SSTR("[Error] provided directive value should be a number '" << *it << "'"));
		i++;
	}
	n = std::atoi(s.c_str());
	if (n < 1024 || n > 65535)
		throw std::runtime_error("[Error] port cannot be greater than range [1024 - 65.535]");
	return n;
}

std::string Configuration::word( std::string raw )
{
	std::istringstream f(raw);
	std::string s, extra = "/.-_'\"";
	size_t i = 0;
	getline(f, s, ' ');
	std::string cmp;
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 256)
			throw std::runtime_error("[Error] excessive number of token characters");
		if (std::isalpha(*it) || (extra.find(*it) != std::string::npos) || (i > 0 && std::isdigit(*it))){}
		else
			throw std::runtime_error("[Error] provided directive value should be a word: '" + raw + "'");
		if (*it != '"')
			cmp.push_back(*it);
		i++;
	}
	if (cmp.size() == 0)
		throw std::runtime_error("[Error] provided empty directive value");
	return cmp;
}

std::string Configuration::path( std::string raw )
{
	std::string s = word(raw);
	if (!(s[0] == '/' || eq(s, ".php") || eq(s, ".py") || eq(s, ".pl")))
		throw std::runtime_error("[Error] paths should be absolute '" + raw + "'");
	return s;
}

std::string Configuration::methods( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	getline(f, s, ' ');
	if (eq(s, "GET")   || eq(s, "POST")    || eq(s, "DELETE") || \
		eq(s, "PUT")   || eq(s, "HEAD")    || eq(s, "OPTIONS") || \
		eq(s, "TRACE") || eq(s, "CONNECT") || eq(s, "PATCH"))
		return s;
	else
		throw std::runtime_error("[Error] unsupported method '" + raw + "'");
}

std::string Configuration::flag( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	getline(f, s, ' ');
	if (eq(s, "on") || eq(s, "off"))
		return s;
	else
		throw std::runtime_error("[Error] unsupported flag value '" + raw + "'");
}

int Configuration::statusCode( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	int n = 0;
	size_t i = 1;
	getline(f, s, ' ');
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (i > 3)
			throw std::runtime_error("[Error] status code cannot be greater than range [100 - 599]");
		if (!std::isdigit(*it))
			throw std::runtime_error("[Error] provided directive value should be a number");
		i++;
	}
	n = std::atoi(s.c_str());
	if (n < 100 || n > 599)
		throw std::runtime_error("[Error] port cannot be greater than range [100 - 599]");
	return n;
}

unsigned long Configuration::number( std::string s )
{
	unsigned long n = 0;
	for(std::string::iterator it = s.begin(); it < s.end(); it++)
	{
		if (!std::isdigit(*it))
			throw std::runtime_error("[Error] provided directive value should be a number");
		if (willMultiplicationOverflow(n, 10))
			throw std::runtime_error("[Error] provided directive value overflows");
		n *= 10;
		if (willAdditionOverflow(n, (*it - '0')))
			throw std::runtime_error("[Error] provided directive value overflows");
		n += (*it - '0');
	}
	return n;
}

unsigned long Configuration::size( std::string raw )
{
	std::istringstream f(raw);
	std::string s;
	getline(f, s, ' ');
	char c = s[s.size() - 1];
	int m = 0;
	
	if (std::isdigit(c))
		m = 1;
	else if (c == 'm')
	{
		m = 1024 * 1024;
		s = s.substr(0, s.find("m"));
	}
	else if (c == 'k')
	{
		m = 1024;
		s = s.substr(0, s.find("k"));
	}
	else
		throw std::runtime_error("[Error] on size format");

	unsigned long n = number(s);

	if (willMultiplicationOverflow(n, m))
		throw std::runtime_error("[Error] provided directive value overflows");

	return n * m;
}

bool Configuration::willAdditionOverflow(unsigned long a, unsigned long b)
{
    return b > std::numeric_limits<unsigned long>::max() - a;
}

bool Configuration::willMultiplicationOverflow(unsigned long a, unsigned long b)
{
    if (a == 0 || b == 0)
        return false;
    return a > std::numeric_limits<unsigned long>::max() / b;
}

bool Configuration::eq( std::string s1, std::string s2 )
{
	return (s1.compare(s2) == 0 && s1.size() == s2.size());
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
ServerList & Configuration::getServerList( void )
{
	return this->_servers;
}

void Configuration::setServer(Server srv)
{
	this->_server = srv;
}

Server Configuration::getServer( void )
{
	return this->_server;
}
