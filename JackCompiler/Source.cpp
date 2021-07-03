#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include "Helpers.hpp"
#include "Tokenizer.hpp"
#include "Parser.hpp"


bool is_directory_jack(std::string s)
{
	if (s.find(".jack") == std::string::npos) {
		return true;
	}
	else return false;
}

bool is_directory_xml(std::string s)
{
	if (s.find(".xml") == std::string::npos) {
		return true;
	}
	else return false;
}

std::string extract_name_windows(std::string const& f)
{
	unsigned int first_pos = f.find_last_of("\\");

	first_pos = first_pos + 1;

	return std::string(f.substr(first_pos, f.size() - first_pos));
}

std::string return_extension(std::string const& f)
{
	unsigned int first_pos = f.find_last_of(".");

	first_pos = first_pos + 1;

	return std::string(f.substr(first_pos, f.size() - first_pos));
}


int main(int argc, char* argv[])
{
	if (is_directory_jack(argv[1])) {
		std::string path(std::filesystem::current_path().string());
		path += "\\" + std::string(argv[1]);
		std::cout << "path: " << path << std::endl;

		for (auto f : std::filesystem::directory_iterator(path)) {
			if (return_extension(f.path().string()) == "jack") {
				Tokenizer tokenizer(f.path().string());
				tokenizer.tokenize();

				Parser parser(extract_name(f.path().string()) + "T.xml");
				parser.parse();
			}
		}
	}
	else {
		Tokenizer tokenizer(argv[1]);
		tokenizer.tokenize();

		Parser parser(std::string(argv[1]) + "T.xml");
		parser.parse();
	}


	std::cout << "Jack Analyzer finished" << std::endl;
}