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
	fo << "<class>" << std::endl;
	inc_indent();

	std::string line;

	auto ignore_line = [this, &line]() { std::getline(fs, line); };

	ignore_line();

	while (std::getline(fs, line)) {
		if (line[1] == '/') continue;
		extract_type_and_token(line);
		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	dec_indent();
	indent();
	fo << "</class>" << std::endl;
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
	fo.open(n + ".xml");
}

void Parser::close_files()
{
	fs.close();
	fo.close();
}

void Parser::inc_indent()
{
	indentation += "  ";
}

void Parser::dec_indent()
{
	Parser::indentation = indentation.substr(0, indentation.size() - 2);
}

void Parser::indent()
{
	fo << indentation;
}


std::string Parser::get_new_token()
{
	std::string line;

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

void Parser::write_line()
{
	indent();
	fo << "<" << type << ">" << " " << token << " " << "</" << type << ">" << std::endl;
}

void Parser::write_next_line(std::string& str)
{
	std::getline(fs, str);
	extract_type_and_token(str);
	write_line();
}


void Parser::write_keyword_class()
{
	write_line();
}

void Parser::write_keyword_method()
{
	indent();
	fo << "<subroutineDec>" << std::endl;
	inc_indent();

	write_line();

	std::string line;

	//keyword return type
	write_next_line(line);

	//keyword function name
	write_next_line(line);

	//open parenthesis
	write_next_line(line);

	indent();
	fo << "<parameterList>" << std::endl;

	inc_indent();
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == ")") {
			break;
		}

		write_line();
	}

	dec_indent();

	indent();
	fo << "</parameterList>" << std::endl;

	//closed parenthesis
	write_line();

	indent();
	fo << "<subroutineBody>" << std::endl;
	inc_indent();

	//open curly braces
	write_next_line(line);

	write_statements();

	//closing brace
	write_line();

	dec_indent();
	indent();
	fo << "</subroutineBody>" << std::endl;

	dec_indent();
	indent();
	fo << "</subroutineDec>" << std::endl;
}

void Parser::write_keyword_function()
{
	indent();
	fo << "<subroutineDec>" << std::endl;
	inc_indent();

	write_line();

	std::string line;

	//keyword return type
	write_next_line(line);

	//keyword function name
	write_next_line(line);

	//open parenthesis
	write_next_line(line);

	indent();
	fo << "<parameterList>" << std::endl;

	inc_indent();
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == ")") {
			break;
		}

		write_line();
	}

	dec_indent();

	indent();
	fo << "</parameterList>" << std::endl;

	//closed parenthesis
	write_line();

	indent();
	fo << "<subroutineBody>" << std::endl;
	inc_indent();

	//open curly braces
	write_next_line(line);

	write_statements();

	//closing brace
	write_line();

	dec_indent();
	indent();
	fo << "</subroutineBody>" << std::endl;

	dec_indent();
	indent();
	fo << "</subroutineDec>" << std::endl;
}

void Parser::write_keyword_constructor()
{
	indent();
	fo << "<subroutineDec>" << std::endl;
	inc_indent();

	write_line();

	std::string line;

	//keyword return type
	write_next_line(line);

	//keyword function name
	write_next_line(line);

	//open parenthesis
	write_next_line(line);

	indent();
	fo << "<parameterList>" << std::endl;

	inc_indent();
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == ")") {
			break;
		}

		write_line();
	}

	dec_indent();

	indent();
	fo << "</parameterList>" << std::endl;

	//closed parenthesis
	write_line();

	indent();
	fo << "<subroutineBody>" << std::endl;
	inc_indent();

	//open curly braces
	write_next_line(line);

	write_statements();

	//closing brace
	write_line();

	dec_indent();
	indent();
	fo << "</subroutineBody>" << std::endl;

	dec_indent();
	indent();
	fo << "</subroutineDec>" << std::endl;
}

void Parser::write_keyword_class_var_dec()
{
	indent();
	fo << "<classVarDec>" << std::endl;
	inc_indent();

	//field or static
	write_line();

	std::string line;
	//type
	write_next_line(line);

	//varname
	write_next_line(line);

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token != ",") {
			break;
		}
		//comma
		write_line();

		//varname
		write_next_line(line);
	}

	//semi-colon;
	write_line();

	dec_indent();
	indent();
	fo << "</classVarDec>" << std::endl;
}

void Parser::write_keyword_int()
{
	write_line();
}

void Parser::write_keyword_boolean()
{
	write_line();
}

void Parser::write_keyword_char()
{
	write_line();
}

void Parser::write_keyword_void()
{
	write_line();
}

void Parser::write_keyword_var()
{
	std::string line;
	indent();
	fo << "<varDec>" << std::endl;
	inc_indent();

	//var
	write_line();

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == ";") {
			break;
		}

		write_line();
	}

	//semi-colon
	write_line();

	dec_indent();
	indent();
	fo << "</varDec>" << std::endl;
}

void Parser::write_keyword_static()
{
	write_line();
}

void Parser::write_keyword_field()
{
	write_line();
}

void Parser::write_var_index()
{
	//identifier
	write_line();
	std::string line;
	//[
	write_next_line(line);

	write_expression();

	//]
	write_line();
}

void Parser::write_subroutine_call()
{
	//identifier
	write_line();

	std::string line;

	std::string new_token = get_new_token();

	if (new_token == ".") {
		//symbol
		write_next_line(line);

		//identifier
		write_next_line(line);

		//open_parenthesis
		write_next_line(line);

		write_expression_list();

		//close_parenthesis
		if (token != "(") {
			write_line();
		}
		else write_next_line(line);
	}
	else if (new_token == "(") {
		//open paren
		write_next_line(line);

		write_expression_list();

		//close_parenthesis
		if (token != "(") {
			write_line();
		}
		else write_next_line(line);
	}
}

void Parser::write_expression_list()
{
	indent();
	fo << "<expressionList>" << std::endl;
	inc_indent();

	std::string new_token = get_new_token();

	if (new_token != ")") {
		write_expression();

		while (token == ",") {
			write_line();
			write_expression();
		}
	}

	dec_indent();
	indent();
	fo << "</expressionList>" << std::endl;
}

void Parser::write_expression()
{
	indent();
	fo << "<expression>" << std::endl;
	inc_indent();

	auto start_term = [this]()
	{
		indent();
		fo << "<term>" << std::endl;
		inc_indent();
	};

	auto end_term = [this]()
	{
		dec_indent();
		indent();
		fo << "</term>" << std::endl;
	};

	std::string line;

	TermType term;
	while (std::getline(fs, line)) {
		prev_token = token;
		extract_type_and_token(line);

		if (token == ",") {
			break;
		}

		if (token == ";") {
			break;
		}

		if (token == ")") {
			break;
		}

		if (token == "]") {
			break;
		}

		term = check_term_type();

		switch (term) {
			case TermType::INTEGER_CONSTANT: {
				start_term();
				write_int_const();
				end_term();
				break;
			}
			case TermType::STRING_CONSTANT: {
				start_term();
				write_string_const();
				end_term();
				break;
			}
			case TermType::VAR: {
				start_term();

				write_identifier();

				end_term();
				break;
			}
			case TermType::VAR_INDEX: {
				start_term();
				write_var_index();
				end_term();
				break;
			}
			case TermType::SUBROUTINE_CALL: {
				start_term();

				write_subroutine_call();

				end_term();
				break;
			}
			case TermType::EXPRESSION: {
				start_term();
				//open_paren
				write_line();
				write_expression();
				//close paren
				write_line();
				end_term();
				break;
			}
			case TermType::UNARY_OP: {
				start_term();
				write_symbol();
				start_term();
				std::string new_token = get_new_token();

				if (new_token == "(") {
					//(
					write_next_line(line);

					//expression
					write_expression();

					write_line();
				}
				else write_next_line(line);
				end_term();
				end_term();
				break;
			}
			case TermType::OP: {
				write_symbol();
				break;
			}
			case TermType::KEYWORD_CONSTANT: {
				start_term();
				write_keyword();
				end_term();
			}
			default:
				break;
		}
	}

	dec_indent();
	indent();
	fo << "</expression>" << std::endl;
}


void Parser::write_keyword_let()
{
	indent();
	fo << "<letStatement>" << std::endl;
	inc_indent();

	//let
	write_line();

	std::string line;
	//varName
	write_next_line(line);

	std::string new_token = get_new_token();
	if (new_token == "[") {
		//[
		write_next_line(line);

		write_expression();

		//]
		write_line();
	}


	write_next_line(line);

	write_expression();

	//semi-colon
	write_line();

	dec_indent();
	indent();
	fo << "</letStatement>" << std::endl;
}

void Parser::write_keyword_do()
{
	indent();
	fo << "<doStatement>" << std::endl;
	inc_indent();

	//do
	write_line();

	std::string line;
	std::getline(fs, line);
	extract_type_and_token(line);

	write_subroutine_call();

	//semicolon
	write_next_line(line);

	dec_indent();
	indent();
	fo << "</doStatement>" << std::endl;
}

void Parser::write_keyword_if()
{
	indent();
	fo << "<ifStatement>" << std::endl;
	inc_indent();

	//if
	write_line();

	std::string line;
	//open paren
	write_next_line(line);

	write_expression();

	//close_paren
	write_line();

	//open curly
	write_next_line(line);

	write_statements();

	//semicolon
	write_line();

	line = get_new_token();
	if (line == "else") {
		write_keyword_else();
	}

	dec_indent();
	indent();
	fo << "</ifStatement>" << std::endl;
}

void Parser::write_keyword_else()
{
	std::string line;

	//else
	write_next_line(line);

	//open curly
	write_next_line(line);

	write_statements();

	//semicolon
	write_line();
}

void Parser::write_keyword_while()
{
	indent();
	fo << "<whileStatement>" << std::endl;
	inc_indent();

	//while
	write_line();

	std::string line;
	//open parenthesis
	write_next_line(line);

	write_expression();

	//close_parenthesis
	write_line();

	//open curly brackets
	write_next_line(line);

	write_statements();

	//close curly brackets
	write_line();

	dec_indent();
	indent();
	fo << "</whileStatement>" << std::endl;
}

void Parser::write_keyword_return()
{
	indent();
	fo << "<returnStatement>" << std::endl;
	inc_indent();

	//return
	write_line();

	std::string new_token = get_new_token();

	if (new_token != ";") {
		write_expression();
		write_line();
	}
	else write_next_line(new_token);

	dec_indent();
	indent();
	fo << "</returnStatement>" << std::endl;
}

void Parser::write_keyword_true()
{
	write_line();
}

void Parser::write_keyword_false()
{
	write_line();
}

void Parser::write_keyword_null_type()
{
	write_line();
}

void Parser::write_keyword_this()
{
	write_line();
}

void Parser::write_keyword()
{
	(this->*(write_functions_keyword_type[static_cast<int>(check_keyword_type())]))();
}

void Parser::write_less_than()
{
	indent();
	fo << "<symbol> &lt; </symbol>" << std::endl;
}

void Parser::write_greater_than()
{
	indent();
	fo << "<symbol> &gt; </symbol>" << std::endl;
}

void Parser::write_equals()
{
	indent();
	fo << "<symbol> &eq; </symbol>" << std::endl;
}

void Parser::write_symbol()
{
	if (token == "<") {
		write_less_than();
	}
	else if (token == ">") {
		write_greater_than();
	}
	else if (token == "==") {
		write_equals();
	}
	write_line();
}

void Parser::write_int_const()
{
	write_line();
}

void Parser::write_string_const()
{
	write_line();
}

void Parser::write_identifier()
{
	write_line();
}

void Parser::write_statements()
{
	std::string line;
	bool initial_statement_indent = false;
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "}") {
			break;
		}

		if (is_statement()) {
			if (initial_statement_indent == false) {
				indent();
				fo << "<statements>" << std::endl;
				inc_indent();
				initial_statement_indent = true;
			}
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	if (initial_statement_indent) {
		dec_indent();
		initial_statement_indent = false;
		indent();
		fo << "</statements>" << std::endl;
	}
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