#include "Parser.hpp"

Parser::Parser(std::string const& filename)
{
	open_files(filename);
	write_functions_vector_init();
	write_functions_keyword_type_vector_init();
}

void Parser::parse()
{
	std::cout << "Parsing " << filename << std::endl;

	auto ignore_line = [this]() { std::getline(fs, line); };

	ignore_line();

	while (std::getline(fs, line)) {
		if (line[1] == '/') continue;
		extract_type_and_token(line);
		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	for (auto& a : symbol_table.subroutine_scope) {
		std::cout << a.first << " " << a.second.index << std::endl;
	}
}

Parser::~Parser()
{
	close_files();
}


void Parser::open_files(std::string const& name)
{
	filename = name;
	fs.open(filename);

	std::string n = extract_name(filename);
	n = n.substr(0, n.size() - 1);
	fo.open(n + ".vm");
}

void Parser::close_files()
{
	fs.close();
	fo.close();
}

std::string Parser::get_new_token()
{
	std::streampos old_pos = fs.tellg();
	std::string old_type = type;
	std::string old_token = token;

	std::getline(fs, line);
	extract_type_and_token(line);
	std::string new_token = token;

	fs.seekg(old_pos);
	token = old_token;
	type = old_type;

	return new_token;
}

void Parser::to_next_token()
{
	std::getline(fs, line);
	extract_type_and_token(line);
}

void Parser::write_keyword_class()
{
	//name
	to_next_token();
	current_class = token;

	//open paren
	to_next_token();
}

void Parser::write_keyword_method()
{

}

void Parser::write_keyword_function()
{
	//func name
	to_next_token();
	fo << "function " << current_class << "." << token << " ";
	std::streampos arg_pos = fo.tellp();
	fo << " " << std::endl;

	std::string line;

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "return") {
			break;
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	std::streampos curr_pos = fo.tellp();

	fo.seekp(arg_pos);
	fo << current_func_args;
	current_func_args = 0;

	fo.seekp(curr_pos);

	write_keyword_return();
}

void Parser::write_keyword_constructor()
{

}

void Parser::write_keyword_class_var_dec()
{

}

void Parser::write_keyword_int()
{
}

void Parser::write_keyword_boolean()
{
}

void Parser::write_keyword_char()
{
}

void Parser::write_keyword_void()
{
}

void Parser::write_keyword_var()
{
	//move to type
	to_next_token();
	std::string type = token;

	SymbolKind kind = SymbolKind::LOCAL;

	//name
	to_next_token();
	if (get_new_token() == ",") {
		while (true) {
			symbol_table.add_symbol(token, type, kind);
			current_func_args++;
			to_next_token();
			if (token != ",") {
				break;
			}
			to_next_token();
		}
	}
	else {
		symbol_table.add_symbol(token, type, kind);
		current_func_args++;
		to_next_token();
	}
}

void Parser::write_keyword_static()
{
	//move to type
	to_next_token();
	std::string type = token;

	SymbolKind kind = SymbolKind::STATIC;

	//name
	to_next_token();
	if (get_new_token() == ",") {
		while (true) {
			symbol_table.add_symbol(token, type, kind);
			to_next_token();
			if (token != ",") {
				break;
			}
			to_next_token();
		}
	}
	else {
		symbol_table.add_symbol(token, type, kind);
		to_next_token();
	}
}

void Parser::write_keyword_field()
{
	//move to type
	to_next_token();
	std::string type = token;

	SymbolKind kind = SymbolKind::FIELD;

	//name
	to_next_token();
	if (get_new_token() == ",") {
		while (true) {
			symbol_table.add_symbol(token, type, kind);
			to_next_token();
			if (token != ",") {
				break;
			}
			to_next_token();
		}
	}
	else {
		symbol_table.add_symbol(token, type, kind);
		to_next_token();
	}
}

void Parser::write_var_index()
{

}

void Parser::write_subroutine_call()
{

}

void Parser::write_expression_list()
{

}

void Parser::write_expression()
{

}


void Parser::write_keyword_let()
{

}

void Parser::write_keyword_do()
{

}

void Parser::write_keyword_if()
{

}

void Parser::write_keyword_else()
{
}

void Parser::write_keyword_while()
{

}

void Parser::write_keyword_return()
{
	fo << "return" << std::endl;
}

void Parser::write_keyword_true()
{

}

void Parser::write_keyword_false()
{

}

void Parser::write_keyword_null_type()
{

}

void Parser::write_keyword_this()
{

}

void Parser::write_keyword()
{
	(this->*(write_functions_keyword_type[static_cast<int>(check_keyword_type())]))();
}

void Parser::write_less_than()
{
}

void Parser::write_greater_than()
{

}

void Parser::write_equals()
{

}

void Parser::write_symbol()
{

}

void Parser::write_int_const()
{

}

void Parser::write_string_const()
{

}

void Parser::write_identifier()
{

}

void Parser::write_statements()
{

}

void Parser::extract_type_and_token(std::string const& str)
{
	if (str == "</tokens>") {
		return;
	}

	size_t token_pos = str.find('>');
	type = str.substr(1, token_pos - 1);

	token_pos += 2;

	token = str.substr(token_pos, str.find_last_of('<') - token_pos - 1);
}

void Parser::write_functions_vector_init()
{
	write_functions_type.resize(static_cast<int>(TokenType::NUM_OF_ITEMS_IN_ENUM));
	write_functions_type[static_cast<int>(TokenType::KEYWORD)] = &Parser::write_keyword;
	write_functions_type[static_cast<int>(TokenType::SYMBOL)] = &Parser::write_symbol;
	write_functions_type[static_cast<int>(TokenType::INTEGER_CONSTANT)] = &Parser::write_int_const;
	write_functions_type[static_cast<int>(TokenType::STRING_CONSTANT)] = &Parser::write_string_const;
	write_functions_type[static_cast<int>(TokenType::IDENTIFIER)] = &Parser::write_identifier;
}

void Parser::write_functions_keyword_type_vector_init()
{
	write_functions_keyword_type.resize(static_cast<int>(KeywordType::NUM_OF_ITEMS_IN_ENUM));
	write_functions_keyword_type[static_cast<int>(KeywordType::CLASS)] = &Parser::write_keyword_class;
	write_functions_keyword_type[static_cast<int>(KeywordType::METHOD)] = &Parser::write_keyword_method;
	write_functions_keyword_type[static_cast<int>(KeywordType::FUNCTION)] = &Parser::write_keyword_function;
	write_functions_keyword_type[static_cast<int>(KeywordType::CONSTRUCTOR)] = &Parser::write_keyword_constructor;
	write_functions_keyword_type[static_cast<int>(KeywordType::INT)] = &Parser::write_keyword_int;
	write_functions_keyword_type[static_cast<int>(KeywordType::BOOLEAN)] = &Parser::write_keyword_boolean;
	write_functions_keyword_type[static_cast<int>(KeywordType::CHAR)] = &Parser::write_keyword_char;
	write_functions_keyword_type[static_cast<int>(KeywordType::VOID)] = &Parser::write_keyword_void;
	write_functions_keyword_type[static_cast<int>(KeywordType::VAR)] = &Parser::write_keyword_var;
	write_functions_keyword_type[static_cast<int>(KeywordType::STATIC)] = &Parser::write_keyword_class_var_dec;
	write_functions_keyword_type[static_cast<int>(KeywordType::FIELD)] = &Parser::write_keyword_class_var_dec;
	write_functions_keyword_type[static_cast<int>(KeywordType::LET)] = &Parser::write_keyword_let;
	write_functions_keyword_type[static_cast<int>(KeywordType::DO)] = &Parser::write_keyword_do;
	write_functions_keyword_type[static_cast<int>(KeywordType::IF)] = &Parser::write_keyword_if;
	write_functions_keyword_type[static_cast<int>(KeywordType::ELSE)] = &Parser::write_keyword_else;
	write_functions_keyword_type[static_cast<int>(KeywordType::WHILE)] = &Parser::write_keyword_while;
	write_functions_keyword_type[static_cast<int>(KeywordType::RETURN)] = &Parser::write_keyword_return;
	write_functions_keyword_type[static_cast<int>(KeywordType::TRUE)] = &Parser::write_keyword_true;
	write_functions_keyword_type[static_cast<int>(KeywordType::FALSE)] = &Parser::write_keyword_false;
	write_functions_keyword_type[static_cast<int>(KeywordType::NULL_TYPE)] = &Parser::write_keyword_null_type;
	write_functions_keyword_type[static_cast<int>(KeywordType::THIS)] = &Parser::write_keyword_this;
}

TokenType Parser::check_type()
{
	if (type == "keyword") {
		return TokenType::KEYWORD;
	}
	else if (type == "identifier") {
		return TokenType::IDENTIFIER;
	}
	else if (type == "symbol") {
		return TokenType::SYMBOL;
	}
	else if (type == "integerConstant") {
		return TokenType::INTEGER_CONSTANT;
	}
	else if (type == "stringConstant") {
		return TokenType::STRING_CONSTANT;
	}
}

KeywordType Parser::check_keyword_type()
{
	if (token == "class") {
		return KeywordType::CLASS;
	}
	else if (token == "method") {
		return KeywordType::METHOD;
	}
	else if (token == "function") {
		return KeywordType::FUNCTION;
	}
	else if (token == "constructor") {
		return KeywordType::CONSTRUCTOR;
	}
	else if (token == "int") {
		return KeywordType::INT;
	}
	else if (token == "bool") {
		return KeywordType::BOOLEAN;
	}
	else if (token == "char") {
		return KeywordType::CHAR;
	}
	else if (token == "void") {
		return KeywordType::VOID;
	}
	else if (token == "var") {
		return KeywordType::VAR;
	}
	else if (token == "static") {
		return KeywordType::STATIC;
	}
	else if (token == "field") {
		return KeywordType::FIELD;
	}
	else if (token == "let") {
		return KeywordType::LET;
	}
	else if (token == "do") {
		return KeywordType::DO;
	}
	else if (token == "if") {
		return KeywordType::IF;
	}
	else if (token == "else") {
		return KeywordType::ELSE;
	}
	else if (token == "while") {
		return KeywordType::WHILE;
	}
	else if (token == "return") {
		return KeywordType::RETURN;
	}
	else if (token == "true") {
		return KeywordType::TRUE;
	}
	else if (token == "false") {
		return KeywordType::FALSE;
	}
	else if (token == "null") {
		return KeywordType::NULL_TYPE;
	}
	else if (token == "this") {
		return KeywordType::THIS;
	}
}

TermType Parser::check_term_type()
{
	TokenType term_type = check_type();

	if (term_type == TokenType::KEYWORD) {
		return TermType::KEYWORD_CONSTANT;
	}

	if (term_type == TokenType::INTEGER_CONSTANT) {
		return TermType::INTEGER_CONSTANT;
	}

	if (term_type == TokenType::STRING_CONSTANT) {
		return TermType::STRING_CONSTANT;
	}

	if (term_type == TokenType::SYMBOL) {
		if (token == "(") {
			return TermType::EXPRESSION;
		}

		if (token == "~") {
			return TermType::UNARY_OP;
		}

		if (token == "-" && (prev_token == "(" || prev_token == "=")) {
			return TermType::UNARY_OP;
		}

		return TermType::OP;
	}

	if (term_type == TokenType::IDENTIFIER) {
		std::string new_token = get_new_token();

		if (new_token == "(" || new_token == ".") {
			return TermType::SUBROUTINE_CALL;
		}
		else if (new_token == "[") {
			return TermType::VAR_INDEX;
		}
		else return TermType::VAR;
	}

	return TermType::NOT_TERM;
}

bool Parser::is_statement()
{
	if (check_type() == TokenType::KEYWORD) {
		KeywordType keyword_type = check_keyword_type();

		switch (keyword_type) {
			case KeywordType::LET:
			case KeywordType::DO:
			case KeywordType::IF:
			case KeywordType::WHILE:
			case KeywordType::RETURN: {
				return true;
			}
			default:
				return false;
		}
	}
	else return false;
}