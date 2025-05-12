#include "Configuration.hpp"

/*
** ------------------------------- CONSTRUCTOR --------------------------------
*/

Configuration::Configuration( std::string const configFile )
{
	this->_pad = "";
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

bool Configuration::isEnding(char c)
{
	return (c=='{' || c=='}' || c==';');
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
		level1.push_back("server_name");
		level1.push_back("root");
		level1.push_back("index");
		level1.push_back("autoindex");
		level1.push_back("return");
		level1.push_back("location");

		level2.push_back("root");
		level2.push_back("index");
		level2.push_back("autoindex");
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
					token.push_back(' ');
					while (read(fd, &c, 1) > 0)
					{
						if (isEnding(c))
						{
							if (c == '{')
								i++;
							else if (c == '}')
								i--;
							break;
						}
						token.push_back(c);
					}
					parseDirective(token);
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

void Configuration::parseDirective( std::string directive )
{
	if (directive.compare("server ") == 0)
	{
		this->_pad = "";
		std::cout << std::endl;
		std::cout << directive << std::endl;
		std::cout << "===================" << std::endl;
	}
	else
	{
		std::cout << this->_pad << directive << std::endl;
		if (directive.find("location") != std::string::npos)
			this->_pad = "  ";
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
