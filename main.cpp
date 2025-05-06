#include "./Connection.hpp"
#include "./Message.hpp"

int main(int argc, char *argv[])
{
	if (argc == 2)
	{
		Connection c(argv[1]);
	}
	else
	{
		std::cerr << "Incorrect number of arguments" << std::endl;
	}
	return (0);
}
