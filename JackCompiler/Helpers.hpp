#pragma once
#include <string>

std::string extract_name(std::string const& f);

size_t precedence(std::string const& op);

std::string to_string_symbol(std::string const& sym);

std::string to_string_unary(std::string const& sym);