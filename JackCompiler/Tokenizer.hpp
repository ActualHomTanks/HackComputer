#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Helpers.hpp"

struct KeyTokens
{
	std::vector<std::string> keywords;
	std::vector<std::string> symbols;

	void read_files(std::string filename);
};

class Tokenizer
{
	std::string filename;
	std::ifstream fs;
	std::ofstream fo;
	std::vector<std::string> tokens;
	KeyTokens key_tokens;

private:
	void open_files(std::string const& name);

	void close_files();

	bool is_symbol(char c) const;

	bool is_keyword(std::string const& key);

	bool is_string_constant(std::string const& key);

	bool is_integer_constant(std::string const& key);

	inline void write_symbol(char c);

	inline void write_keyword(std::string const& keyword);

	inline void write_integer(std::string const& integer);

	inline void write_identifier(std::string const& identifier);

	inline void write_string(std::string const& str);

	void write_token(std::string const& token);

public:
	Tokenizer(std::string filename);

	void set_new_file(std::string name);

	void tokenize();

	~Tokenizer();
};