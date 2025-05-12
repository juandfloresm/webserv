#ifndef CONFIG_PARSER_HPP
# define CONFIG_PARSER_HPP

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <sstream>

class ServerBlock;
class LocationBlock;

typedef std::map<std::string, std::string> Directive;
typedef std::vector<ServerBlock> ServerBlocks;
typedef std::vector<LocationBlock> LocationBlocks;

class LocationBlock {
	public:
		LocationBlock(const std::string& path);
		~LocationBlock();

		std::string getPath() const;
		std::string getValue(const std::string& key) const;
		bool hasKey(const std::string& key) const;

		void addDirective(const std::string& key, const std::string& value);

		Directive& getDirective();
		const Directive& getDirective() const;

		bool isRegex() const { return _isRegex; }
		void setRegex(bool value) { _isRegex = value; }
	
	private:
		std::string _path;
		Directive _directives;
		bool _isRegex;
};

class ServerBlock {
	public:
		ServerBlock();
		~ServerBlock();

		void addDirective(const std::string& key, const std::string& value);
		void addLocationBlock(const LocationBlock& location);

		std::string getValue(const std::string& key) const;
		bool hasKey(const std::string& key) const;
		LocationBlock* getLocation(const std::string& path);
		const LocationBlocks& getLocations() const;
		Directive& getDirectives();
		const Directive& getDirectives() const;

	private:
		Directive _directives;
		LocationBlocks _locations;
};

class ConfigParser {
	public:
		ConfigParser(const std::string& filePath);
		~ConfigParser();

		void parse();
		const ServerBlocks& getServerBlocks() const;
	
	private:
		enum TokenType {
			WORD,
			OPEN_BLOCK,
			CLOSE_BLOCK,
			SEMICOLON,
			END_OF_FILE,
			INVALID
		};

		struct Token {
			TokenType type;
			std::string value;
		};

		std::string _filePath;
		std::ifstream _file;
		ServerBlocks _serverBlocks;

		Token getNextToken();
		void parseServerBlock();
		void parseLocationBlock(ServerBlock& server);
		void parseDirective(Directive& directive);
		void skipWhitespace();
		void throwError(const std::string& message);
};

#endif