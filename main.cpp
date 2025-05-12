#include "./Connection.hpp"
#include "./Message.hpp"
#include "./Configuration.hpp"

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		Configuration("./config/new.conf");
		Connection c(argv[1]);
	}
	else
	{
		std::cerr << "Incorrect number of arguments" << std::endl;
	}
	return (0);
}
