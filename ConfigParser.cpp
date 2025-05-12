#include "ConfigParser.hpp"

LocationBlock::LocationBlock(const std::string& path) : _path(path), _isRegex(false) {}

LocationBlock::~LocationBlock() {}

std::string LocationBlock::getPath() const {
	return _path;
}

std::string LocationBlock::getValue(const std::string& key) const {
	Directive::const_iterator it = _directives.find(key);
	if (it != _directives.end())
		return it->second;
	return "";
}

bool LocationBlock::hasKey(const std::string& key) const {
	return _directives.find(key) != _directives.end();
}

void LocationBlock::addDirective(const std::string& key, const std::string& value) {
	_directives[key] = value;
}

Directive& LocationBlock::getDirective() {
	return this->_directives;
}

const Directive& LocationBlock::getDirective() const {
	return this->_directives;
}

// put into individual files later
// Server Block

ServerBlock::ServerBlock() {}

ServerBlock::~ServerBlock() {}

void ServerBlock::addDirective(const std::string& key, const std::string& value) {
	_directives[key] = value;
}

void ServerBlock::addLocationBlock(const LocationBlock& location) {
	_locations.push_back(location);
}

std::string ServerBlock::getValue(const std::string& key) const {
	Directive::const_iterator it = _directives.find(key);
	if (it != _directives.end())
		return it->second;
	return "";
}

bool ServerBlock::hasKey(const std::string& key) const {
	return _directives.find(key) != _directives.end();
}

LocationBlock* ServerBlock::getLocation(const std::string& path) {
	for (LocationBlocks::iterator it = _locations.begin(); it != _locations.end(); ++it) {
		if (it->getPath() == path) {
			return &(*it);
		}
	}
	return NULL;
}

const LocationBlocks& ServerBlock::getLocations() const {
	return _locations;
}

Directive& ServerBlock::getDirectives() {
	return this->_directives;
}

const Directive& ServerBlock::getDirectives() const {
	return this->_directives;
}



// Config Parser

ConfigParser::ConfigParser(const std::string& filePath) : _filePath(filePath) {
	_file.open(filePath.c_str());
	if (!_file.is_open()) {
		throwError("Could not open config file: " + filePath);
	}
}

ConfigParser::~ConfigParser() {
	if (_file.is_open())
		_file.close();
}

void ConfigParser::parse() {
	Token token;

	while (true) {
		token = getNextToken();

		if (token.type == END_OF_FILE)
			break;

		if (token.type != WORD)
			throwError("Expected directive or block name, got: " + token.value);
		
		if (token.value == "server") {
			parseServerBlock();
		} else {
			throwError("Unknown directive at root level: " + token.value);
		}
	}

	if (_serverBlocks.empty())
		throwError("No server blocks found in configuration");
}

ConfigParser::Token ConfigParser::getNextToken() {
	Token token;
	char c;
	
	skipWhitespace();
	
	if (_file.eof()) {
		token.type = END_OF_FILE;
		return token;
	}
	
	c = _file.get();
	
	if (c == '{') {
		token.type = OPEN_BLOCK;
		token.value = "{";
	}
	else if (c == '}') {
		token.type = CLOSE_BLOCK;
		token.value = "}";
	}
	else if (c == ';') {
		token.type = SEMICOLON;
		token.value = ";";
	}
	else if (isalnum(c) || c == '_' || c == '/' || c == '.' || c == '-' || c == '~' || c == '\\' || c == '$' || c == '^') {
		token.type = WORD;
		token.value = c;
		
		while (_file.good()) {
			c = _file.peek();
			if (isalnum(c) || c == '_' || c == '/' || c == '.' || c == '-' || c == ':' || 
				c == '\\' || c == '$' || c == '^' || c == '*' || c == '+' || c == '?' || c == '|') {
				_file.get();
				token.value += c;
			} else {
				break;
			}
		}
	}
	else {
		token.type = INVALID;
		token.value = c;
	}
	
	return token;
}

void ConfigParser::parseServerBlock() {
	Token token = getNextToken();
	if (token.type != OPEN_BLOCK) {
		throwError("Expected { after server, got: " + token.value);
	}

	ServerBlock server;

	while (true) {
		token = getNextToken();

		if (token.type == CLOSE_BLOCK)
			break;
		
		if (token.type != WORD)
			throwError("Expected directive or block name, got: " + token.value);
		
		if (token.value == "location") {
			parseLocationBlock(server);
		} else {
			_file.unget();
			for (size_t i = 1; i < token.value.length(); i++)
				_file.unget();
			parseDirective(server.getDirectives());
		}
	}

	_serverBlocks.push_back(server);
}

void ConfigParser::parseLocationBlock(ServerBlock& server) {
	Token token = getNextToken();
	bool isRegex = false;
	
	if (token.type == WORD && token.value == "~") {
		isRegex = true;
		token = getNextToken();
		
		if (token.type != WORD)
			throwError("Expected regex pattern after ~, got: " + token.value);
	}
	
	if (token.type != WORD)
		throwError("Expected location path, got: " + token.value);
		
	std::string path = token.value;
	
	token = getNextToken();
	if (token.type != OPEN_BLOCK)
		throwError("Expected { after location " + path + ", got: " + token.value);
		
	LocationBlock location(path);
	location.setRegex(isRegex);
	
	while (true) {
		token = getNextToken();
		
		if (token.type == CLOSE_BLOCK)
			break;
			
		if (token.type != WORD)
			throwError("Expected directive, got: " + token.value);
			
		_file.unget();
		for (size_t i = 1; i < token.value.length(); i++)
			_file.unget();
			
		parseDirective(location.getDirective());
	}
	
	server.addLocationBlock(location);
}

void ConfigParser::parseDirective(Directive& directives) {
	Token token = getNextToken();
	if (token.type != WORD)
		throwError("Expected directive name, got: " + token.value);
		
	std::string name = token.value;
	std::string value = "";
	
	token = getNextToken();
	while (token.type == WORD) {
		value += token.value + " ";
		token = getNextToken();
	}
	
	if (token.type != SEMICOLON)
		throwError("Expected ; after directive value, got: " + token.value);
		
	if (!value.empty() && value[value.length() - 1] == ' ')
		value = value.substr(0, value.length() - 1);
		
	directives[name] = value;
}

void ConfigParser::skipWhitespace() {
	char c;

	while (_file.good() && !_file.eof()) {
		c = _file.peek();

		if (isspace(c)) {
			_file.get();
			continue;
		}

		if (c == '#') {
			_file.get();
			while (_file.good() && !_file.eof()) {
				c = _file.get();
				if (c == '\n')
					break;
			}
			continue;
		}
		break;
	}
}

void ConfigParser::throwError(const std::string& message) {
	throw std::runtime_error("Config parse error: " + message);
}

const ServerBlocks& ConfigParser::getServerBlocks() const {
	return _serverBlocks;
}
