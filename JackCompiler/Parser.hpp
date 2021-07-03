#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

#include "Helpers.hpp"

enum class TokenType
{
	KEYWORD,
	SYMBOL,
	INTEGER_CONSTANT,
	STRING_CONSTANT,
	IDENTIFIER,
	NUM_OF_ITEMS_IN_ENUM,
};

enum class KeywordType
{
	CLASS,
	METHOD,
	FUNCTION,
	CONSTRUCTOR,
	INT,
	BOOLEAN,
	CHAR,
	VOID,
	VAR,
	STATIC,
	FIELD,
	LET,
	DO,
	IF,
	ELSE,
	WHILE,
	RETURN,
	TRUE,
	FALSE,
	NULL_TYPE,
	THIS,
	NUM_OF_ITEMS_IN_ENUM,
};

enum class TermType
{
	NOT_TERM,
	INTEGER_CONSTANT,
	STRING_CONSTANT,
	KEYWORD_CONSTANT,
	VAR,
	VAR_INDEX,
	SUBROUTINE_CALL,
	EXPRESSION,
	UNARY_OP,
	OP,
	NUM_OF_ITEMS_IN_ENUM,
};

class Parser
{
	std::string filename;
	std::ifstream fs;
	std::ofstream fo;
	std::vector<void(Parser::*)()> write_functions_type;
	std::vector<void(Parser::*)()> write_functions_keyword_type;
	std::string type;
	std::string token;
	std::string prev_token;
	std::string indentation = "";

private:
	void open_files(std::string const& name);

	void close_files();

	void inc_indent();

	void dec_indent();

	void indent();

	std::string get_new_token();

	void write_line();

	void write_next_line(std::string& str);

	void write_keyword_class();

	void write_keyword_method();

	void write_keyword_function();

	void write_keyword_constructor();

	void write_keyword_class_var_dec();

	void write_keyword_int();

	void write_keyword_boolean();

	void write_keyword_char();

	void write_keyword_void();

	void write_keyword_var();

	void write_keyword_static();

	void write_keyword_field();

	void write_var_index();

	void write_subroutine_call();

	void write_expression_list();

	void write_expression();

	void write_keyword_let();

	void write_keyword_do();

	void write_keyword_if();

	void write_keyword_else();

	void write_keyword_while();

	void write_keyword_return();

	void write_keyword_true();

	void write_keyword_false();

	void write_keyword_null_type();

	void write_keyword_this();

	void write_keyword();

	void write_less_than();

	void write_greater_than();

	void write_equals();

	void write_symbol();

	void write_int_const();

	void write_string_const();

	void write_identifier();

	void write_statements();

	void extract_type_and_token(std::string const& str);

	void write_functions_vector_init();

	void write_functions_keyword_type_vector_init();

	TokenType check_type();

	KeywordType check_keyword_type();

	TermType check_term_type();

	bool is_statement();

public:
	Parser(std::string const& filename);

	void parse();

	~Parser();
};