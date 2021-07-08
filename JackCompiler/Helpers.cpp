#include <iostream>
#include "Helpers.hpp"

std::string extract_name(std::string const& f)
{
	int pos = f.find(".");
	return std::string(f.substr(0, pos));
}

size_t precedence(std::string const& op)
{
	if (op == "=") {
		return 0;
	}
	if (op == "+" || op == "-") {
		return 1;
	}
	if (op == "*" || op == "/") {
		return 2;
	}
	else std::cout << "invalid operand " << op << std::endl;
}

std::string to_string_symbol(std::string const& sym)
{
	if (sym == "+") {
		return "add";
	}

	if (sym == "-") {
		return "sub";
	}

	if (sym == "*") {
		return "call Math.multiply 2";
	}

	if (sym == "/") {
		return "call Math.divide 2";
	}

	if (sym == "=") {
		return "eq";
	}

	if (sym == "~") {
		return "not";
	}
}

std::string to_string_unary(std::string const& sym)
{
	if (sym == "-") {
		return "neg";
	}

	if (sym == "~") {
		return "not";
	}

}
