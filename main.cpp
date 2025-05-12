#include "./Connection.hpp"
#include "./Message.hpp"
#include "./Configuration.hpp"

int main(int argc, char *argv[])
{
	if (argc == 2) {
		try {
			std::cout << "Starting server with configuration: " << argv[1] << std::endl;
			Configuration cf("./config/new.conf");
			Connection c(argv[1]);
		}
		catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
	}
	else {
		std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
		std::cerr << "Example: " << argv[0] << " ./config/sample.conf" << std::endl;
	}

	return 0;
}
