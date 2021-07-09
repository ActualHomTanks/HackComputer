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
	size_t class_static_index = 0;
	size_t class_field_index = 0;
	size_t subroutine_argument_index = 0;
	size_t subroutine_local_index = 0;

public:
	void add_symbol(std::string const& n, std::string const& t, SymbolKind k)
	{
		if (k == SymbolKind::STATIC ) {
			Info new_info(class_static_index, k, t);

			class_scope.insert(std::make_pair(n, new_info));
			class_static_index++;
		}
		else if (k == SymbolKind::FIELD) {
			Info new_info(class_field_index, k, t);

			class_scope.insert(std::make_pair(n, new_info));
			class_field_index++;
		}
		else if (k == SymbolKind::LOCAL) {
			Info new_info(subroutine_local_index, k, t);

			subroutine_scope.insert(std::make_pair(n, new_info));
			subroutine_local_index++;
		}
		else {
			Info new_info(subroutine_argument_index, k, t);

			subroutine_scope.insert(std::make_pair(n, new_info));
			subroutine_argument_index++;
		}
	}

	void clear_subroutine()
	{
		subroutine_scope.clear();
		subroutine_local_index = 0;
		subroutine_argument_index = 0;
	}

	void clear_class()
	{
		class_scope.clear();
		class_field_index = 0;
		class_static_index = 0;
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
			return Info(0, SymbolKind::NUM_OF_ITEMS_IN_ENUM, "invalid");
		}
	}

	std::string static kind_to_string(SymbolKind k)
	{
		if (k == SymbolKind::STATIC) {
			return "static";
		}
		else if (k == SymbolKind::FIELD) {
			return "this";
		}
		else if (k == SymbolKind::ARG) {
			return "argument";
		}
		else if (k == SymbolKind::LOCAL) {
			return "local";
		}
	}
};