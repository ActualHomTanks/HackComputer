#pragma once
#include <unordered_map>
#include <iostream>

enum class SymbolKind
{
	STATIC,
	FIELD,
	ARG,
	LOCAL,
	NUM_OF_ITEMS_IN_ENUM
};

struct Info
{
	size_t index;
	SymbolKind kind;
	std::string type;

	Info(size_t index, SymbolKind kind, std::string const& type): index(index), kind(kind), type(type)
	{}
};

class SymbolTable
{
public:
	std::unordered_map<std::string, Info> class_scope;
	std::unordered_map<std::string, Info> subroutine_scope;
	size_t class_index = 0;
	size_t subroutine_index = 0;

public:
	void add_symbol(std::string const& n, std::string const& t, SymbolKind k)
	{
		if (k == SymbolKind::STATIC || k == SymbolKind::FIELD) {
			Info new_info(class_index, k, t);

			class_scope.insert(std::make_pair(n, new_info));
			class_index++;
		}
		else {
			Info new_info(subroutine_index, k, t);

			subroutine_scope.insert(std::make_pair(n, new_info));
			subroutine_index++;
		}
	}

	void clear_subroutine()
	{
		subroutine_scope.clear();
		subroutine_index = 0;
	}

	void clear_class()
	{
		class_scope.clear();
		class_index = 0;
	}

	Info const& return_info(std::string const& n)
	{
		if (subroutine_scope.find(n) != subroutine_scope.end()) {
			return subroutine_scope.at(n);
		}
		else if (class_scope.find(n) != class_scope.end()) {
			return class_scope.at(n);
		}
		else {
			std::cout << "SYMBOL NOT FOUND" << std::endl;
		}
	}
};