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

std::string Configuration::parse( std::string const file )
{
	int fd = open(file.c_str(), O_RDONLY);
	if (fd < 0)
	{
		throw std::exception();
	}
	else 
	{
		char c;
		std::map<int, std::vector<std::string> > levels;
		std::vector<std::string> level0, level1;
		level0.push_back("server");
		level0.push_back("{");

		level1.push_back("listen");
		level1.push_back("server_name");
		level1.push_back("root");
		level1.push_back("index");
		level1.push_back("autoindex");
		level1.push_back("return");
		level1.push_back("}");

		levels[0] = level0;
		levels[1] = level1;
		int i = 0;

		std::string token = "";
		while (read(fd, &c, 1) > 0)
		{
			if (c==' ' || c=='\n' || c=='\t' || c=='\v' || c=='\r' || c=='\f')
			{
				if (token.size() > 0)
				{
					bool found = false;
					for (size_t j = 0; j < levels[i].size(); j++)
					{
						if (token.compare(levels[i][j]) == 0)
						{
							found = true;
							break;
						}
					}
					if (found)
					{
						if (i == 1 && token.compare("}") != 0)
						{
							token.push_back(' ');
							bool compute = false;
							while (read(fd, &c, 1) > 0)
							{
								if (c == ';')
									break;
								if (c == ' ' && !compute)
									continue ;
								compute = true;
								token.push_back(c);
							}
						}
						
						if (token.compare("{") == 0)
							i++;
						else if (token.compare("}") == 0)
							i--;
						else
							parseDirective(token);
					}
					else
					{
						std::cout << "Could not find (" << i << ")" << token << std::endl;
						throw std::exception();
					}
				}
				token = "";
			}
			else
				token.push_back(c);
		}
	}
	return "";
}

void Configuration::parseDirective( std::string directive )
{
	if (directive.compare("server") != 0)
	{
		std::cout << directive << std::endl;
	}
}

/*
** --------------------------------- ACCESSOR ---------------------------------
*/
