#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration( std::string const configFile ) 
{
	parse(configFile);
}

/*
** -------------------------------- DESTRUCTOR --------------------------------
*/

Configuration::~Configuration()
{

}


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
	if (c=='{' || c=='}' || c==';')
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

		std::map<int, std::vector<std::string> > levels;
		std::vector<std::string> level0, level1, level2;

		level0.push_back("server");

		level1.push_back("listen");
		level1.push_back("root");
		level1.push_back("server_name");
		level1.push_back("index");
		level1.push_back("error_page");
		level1.push_back("return");
		level1.push_back("autoindex");

		level1.push_back("location");

		level2.push_back("root");
		level2.push_back("index");
		level2.push_back("autoindex");
		level2.push_back("error_page");
		level2.push_back("return");

		levels[0] = level0;
		levels[1] = level1;
		levels[2] = level2;

		int i = 0;

		std::string token = "";
		while (read(fd, &c, 1) > 0)
		{
			if (isSpace(c))
			{
				if (token.compare("{") == 0)
					i++;
				else if (token.compare("}") == 0)
					i--;
				else if (isTokenValid(i, token, levels))
				{
					std::string value = "";
					while (read(fd, &c, 1) > 0)
					{
						if (isEnding(c, &i))
							break;
						value.push_back(c);
					}
					parseEntry(std::make_pair(token, value));
				}
				else if(token.size() > 0)
				{
					std::cerr << "[Error] Invalid token: [" << token << "]" << std::endl;
					throw std::exception();
				}
				token = "";
			}
			else
				token.push_back(c);
		}
	}
}


bool Configuration::isTokenValid(int i, std::string token, std::map<int, std::vector<std::string> > levels)
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
		this->_servers.back().setLocation(Location());
		this->_parsingServer = false;
	}
	else if (this->_parsingServer)
		parseContext(this->_servers.back(), directive);
	else
		parseContext(this->_servers.back().getLocations().back(), directive);
}

void Configuration::parseContext( Context & cxt, Entry directive )
{
	if (directive.first.compare("listen") == 0)
		cxt.setPort(std::atoi(directive.second.c_str()));
	else if (directive.first.compare("path") == 0)
		cxt.setPath(directive.second);
	else if (directive.first.compare("root") == 0)
		cxt.setRoot(directive.second);
	else if (directive.first.compare("server_name") == 0)
	{
		std::istringstream f(directive.second);
		std::string s;
		while (getline(f, s, ' '))
			cxt.setServerName(s);
	}
	else if (directive.first.compare("error_page") == 0)
	{
		std::istringstream f(directive.second);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		cxt.setErrorPage(atoi(code.c_str()), page);
	}
	else if (directive.first.compare("return") == 0)
	{
		std::istringstream f(directive.second);
		std::string code, page;
		getline(f, code, ' ');
		getline(f, page, ' ');
		cxt.setReturn(atoi(code.c_str()), page);
	}
	else if (directive.first.compare("index") == 0)
	{
		std::istringstream f(directive.second);
		std::string s;
		cxt.getIndex().clear();
		while (getline(f, s, ' '))
			cxt.setIndex(s);
	}
	else if (directive.first.compare("autoindex") == 0)
	{
		if (directive.second.compare("on") == 0)
			cxt.setAutoIndex(true);
		else
			cxt.setAutoIndex(false);
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
ServerList & Configuration::getServerList( void )
{
	return this->_servers;
}
