#include "Helpers.hpp"

std::string extract_name(std::string const& f)
{
	int pos = f.find(".");
	return std::string(f.substr(0, pos));
}