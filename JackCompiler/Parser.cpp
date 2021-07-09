#include <iostream>
#include <sstream>
#include <stack>
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

	name_without_extension = extract_name(filename);
	name_without_extension = name_without_extension.substr(0, name_without_extension.size() - 1);
	fo.open(name_without_extension + ".vm");

	name_without_extension = name_without_extension.substr(name_without_extension.find_last_of("\\") + 1);
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
	std::string old_line = line;
	std::string old_prev = prev_token;

	std::getline(fs, line);
	extract_type_and_token(line);
	std::string new_token = token;

	fs.seekg(old_pos);
	token = old_token;
	type = old_type;
	line = old_line;
	prev_token = old_prev;

	return new_token;
}

void Parser::to_next_token()
{
	prev_token = token;
	std::getline(fs, line);
	if (line != "") {
		extract_type_and_token(line);
	}
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
	//return type
	to_next_token();

	bool is_void = false;
	if (token == "void") {
		is_void = true;
	}

	//meth_name
	to_next_token();
	std::string current_name = token;
	std::cout << "method " << token << std::endl;
	fo << "function " << current_class << "." << token << " ";
	std::streampos arg_pos = fo.tellp();
	fo << " " << std::endl;



	//open paren
	to_next_token();

	symbol_table.add_symbol("this", current_class, SymbolKind::ARG);

	if (get_new_token() != ")") {
		do {
			extract_type_and_token(line);
			if (token == ")") {
				break;
			}

			//move to type
			to_next_token();
			std::string type = token;

			SymbolKind kind = SymbolKind::ARG;

			//name
			to_next_token();

			symbol_table.add_symbol(token, type, kind);

		} while (std::getline(fs, line));

	}

	for (auto& a : symbol_table.subroutine_scope) {
		std::cout << "function arg " << a.first << " " << a.second.type << std::endl;
	}

	fo << "push argument 0" << std::endl;
	fo << "pop pointer 0" << std::endl;

	//open curly paren
	to_next_token();

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "return") {
			break;
		}

		std::string new_token = get_new_token();
		if (new_token == "function" || new_token == "method") {
			break;
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	std::streampos curr_pos = fo.tellp();

	fo.seekp(arg_pos);
	fo << current_func_args;
	current_func_args = 0;

	fo.seekp(curr_pos);

	if (is_void) {
		fo << "push constant 0" << std::endl;
	}

	write_keyword_return();
	symbol_table.clear_subroutine();
}

void Parser::write_keyword_function()
{
	//func name
	to_next_token();

	bool is_void = false;
	if (token == "void") {
		is_void = true;
	}

	to_next_token();
	std::string current_name = token;
	std::cout << "function " << token << std::endl;
	fo << "function " << current_class << "." << token << " ";
	std::streampos arg_pos = fo.tellp();
	fo << " " << std::endl;

	//open paren
	to_next_token();

	if (get_new_token() != ")") {
		do {
			extract_type_and_token(line);
			if (token == ")") {
				break;
			}

			//move to type
			to_next_token();
			std::string type = token;

			SymbolKind kind = SymbolKind::ARG;

			//name
			to_next_token();

			symbol_table.add_symbol(token, type, kind);

		} while (std::getline(fs, line));

	}

	for (auto& a : symbol_table.subroutine_scope) {
		std::cout << "function arg " << a.first << " " << a.second.type << std::endl;
	}

	//open curly paren
	to_next_token();

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "return") {
			break;
		}

		std::string new_token = get_new_token();
		if (new_token == "function" || new_token == "method") {
			break;
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	std::streampos curr_pos = fo.tellp();

	fo.seekp(arg_pos);
	fo << current_func_args;
	current_func_args = 0;

	fo.seekp(curr_pos);

	if (is_void) {
		fo << "push constant 0" << std::endl;
	}

	write_keyword_return();
	symbol_table.clear_subroutine();
}

void Parser::write_keyword_constructor()
{
	//return type
	to_next_token();

	//func name
	to_next_token();
	std::string current_name = token;
	std::cout << "constructor " << token << std::endl;
	fo << "function " << current_class << "." << token << " ";
	std::streampos arg_pos = fo.tellp();
	fo << " " << std::endl;

	//open paren
	to_next_token();

	if (get_new_token() != ")") {
		do {
			extract_type_and_token(line);
			if (token == ")") {
				break;
			}

			//move to type
			to_next_token();
			std::string type = token;

			SymbolKind kind = SymbolKind::ARG;

			//name
			to_next_token();

			symbol_table.add_symbol(token, type, kind);

		} while (std::getline(fs, line));

	}

	for (auto& a : symbol_table.subroutine_scope) {
		std::cout << "function arg " << a.first << " " << a.second.type << std::endl;
	}

	for (auto& a : symbol_table.class_scope) {
		std::cout << "class arg " << a.first << " " << a.second.type << " " << a.second.index << std::endl;
	}

	fo << "push constant " << symbol_table.class_field_index << std::endl;

	fo << "call Memory.alloc 1" << std::endl;

	fo << "pop pointer 0" << std::endl;

	//open curly paren
	to_next_token();

	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "return") {
			break;
		}

		std::string new_token = get_new_token();
		if (new_token == "function" || new_token == "method") {
			break;
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}

	std::streampos curr_pos = fo.tellp();

	fo.seekp(arg_pos);
	fo << current_func_args;
	current_func_args = 0;

	fo.seekp(curr_pos);

	/*if (is_void) {
		fo << "push constant 0" << std::endl;
	}*/

	write_keyword_return();
	symbol_table.clear_subroutine();
}

void Parser::write_keyword_class_var_dec()
{
	std::cout << "";
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
	//identifier or class name if method
	std::string t = token;

	std::string new_token = get_new_token();

	if (new_token == ".") {
		//.
		to_next_token();

		//meth name
		to_next_token();

		std::string meth_name = token;

		//open paren
		to_next_token();

		size_t args = 0;
		std::string new_token = get_new_token();

		if (new_token != ")") {
			write_expression();
			args++;

			while (token == ",") {
				write_expression();
				args++;
			}
		}

		//close paren
		//to_next_token();

		Info const& info = symbol_table.return_info(t);

		//symbol not found case
		if (info.kind == SymbolKind::NUM_OF_ITEMS_IN_ENUM) {
			fo << "call " << t << "." << meth_name << " " << args << std::endl;
		}
		else {
			fo << "push " << SymbolTable::kind_to_string(info.kind) << " " << info.index << std::endl;
			fo << "call " << info.type << "." << meth_name << " " << args + 1 << std::endl;
		}

	}
	else if (new_token == "(" && is_do == false) {
		//open paren
		to_next_token();

		size_t args = 0;
		std::string new_token = get_new_token();

		if (new_token != ")") {
			write_expression();
			args++;

			while (token == ",") {
				write_expression();
				args++;
			}
		}

		//close paren
		to_next_token();

		fo << "call " << t << " " << args << std::endl;
	}
	else if (new_token == "(" && is_do == true) {
		//open paren
		to_next_token();

		size_t args = 0;
		std::string new_token = get_new_token();

		if (new_token != ")") {
			write_expression();
			args++;

			while (token == ",") {
				write_expression();
				args++;
			}
		}

		//close paren
		to_next_token();

		fo << "push pointer 0" << std::endl;
		fo << "call " << current_class << "." << t << " " << args + 1 << std::endl;
	}
}

void Parser::write_expression_list()
{

}

void Parser::write_expression()
{
	//int outvar = 0;

	std::stack<std::string> operands;
	if (token == "(") {
		operands.push(token);
	}

	TermType term;
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == ",") {
			while (!operands.empty() && operands.top() != "(") {
				fo << to_string_symbol(operands.top()) << std::endl;
				operands.pop();
			}
			if (!operands.empty()) {
				operands.pop();
			}
			break;
		}

		if (token == ";") {
			while (!operands.empty() && operands.top() != "(") {
				fo << to_string_symbol(operands.top()) << std::endl;
				operands.pop();
			}
			if (!operands.empty()) {
				operands.pop();
			}
			break;
		}

		if (!is_conditional) {
			if (token == "=") {
				while (!operands.empty() && operands.top() != "(") {
					fo << to_string_symbol(operands.top()) << std::endl;
					operands.pop();
				}
				if (!operands.empty()) {
					operands.pop();
				}
				break;
			}
		}

		if (token == ")") {
			while (!operands.empty() && operands.top() != "(") {
				fo << to_string_symbol(operands.top()) << std::endl;
				operands.pop();
			}
			if (!operands.empty()) {
				operands.pop();
			}
			break;
		}

		if (token == "]") {
			while (!operands.empty() && operands.top() != "(") {
				fo << to_string_symbol(operands.top()) << std::endl;
				operands.pop();
			}
			if (!operands.empty()) {
				operands.pop();
			}
			break;
		}

		if (token == "|") {
			std::cout << "";
		}

		term = check_term_type();

		switch (term) {
			case TermType::INTEGER_CONSTANT: {
				fo << "push constant " << token << std::endl;
				break;
			}
			case TermType::STRING_CONSTANT: {
				fo << "push constant " << token.size() << std::endl;
				fo << "call String.new 1" << std::endl;
				for (char c : token) {
					fo << "push constant " << static_cast<int>(c) << std::endl;
					fo << "call String.appendChar 2" << std::endl;
				}
				break;
			}
			case TermType::VAR: {
				Info const& info = symbol_table.return_info(token);

				fo << "push " << SymbolTable::kind_to_string(info.kind) << " " << info.index << std::endl;

				break;
			}
			case TermType::VAR_INDEX: {
				//check for expression type
				/*TermType tt = check_term_type();
				if (tt == TermType::INTEGER_CONSTANT) {

				}*/
				Info const& info = symbol_table.return_info(token);
				fo << "push " << SymbolTable::kind_to_string(info.kind) << " " << info.index << std::endl;

				//[
				to_next_token();

				write_expression();


				//if (token == "=" || new_token == "=") {
				//	while (!operands.empty() && operands.top() != "(") {
				//		fo << to_string_symbol(operands.top()) << std::endl;
				//		operands.pop();
				//	}
				//	if (!operands.empty()) {
				//		operands.pop();
				//	}
				//	//outvar = 1;
				//	//fo << "add" << std::endl;
				//	if (new_token != "=") {
				//		to_next_token();
				//	}
				//	return;
				//}

				std::string new_token = get_new_token();
				token;

				if (token == "]" || token == ")" || token == ";" || token == "=") {
					/*while (!operands.empty() && operands.top() != "(") {
						fo << to_string_symbol(operands.top()) << std::endl;
						operands.pop();
					}
					if (!operands.empty()) {
						operands.pop();
					}*/
					//outvar = 1;
					fo << "add" << std::endl;

					if (new_token != "=") {
						fo << "pop pointer 1" << std::endl;
						fo << "push that 0" << std::endl;
						while (!operands.empty() && operands.top() != "(") {
							fo << to_string_symbol(operands.top()) << std::endl;
							operands.pop();
						}
						if (!operands.empty()) {
							operands.pop();
						}
					}
					else {
						while (!operands.empty() && operands.top() != "(") {
							fo << to_string_symbol(operands.top()) << std::endl;
							operands.pop();
						}
						if (!operands.empty()) {
							operands.pop();
						}
						//to_next_token();
						return;
					}
					//to_next_token();

					//break;
				}

				break;
			}
			case TermType::SUBROUTINE_CALL: {
				write_subroutine_call();
				break;
			}
			case TermType::EXPRESSION: {
				write_expression();
				break;
			}
			case TermType::UNARY_OP: {
				std::string symbol = token;
				std::string new_token = get_new_token();

				if (new_token == "(") {
					//(
					to_next_token();

					//expression
					write_expression();
				}
				else {
					write_expression();

				}

				fo << to_string_unary(symbol) << std::endl;
				break;
				//std::string t = token;

				//std::string new_token = get_new_token();
				//if (new_token == "(") {
				//	//(
				//	to_next_token();
				//	
				//	//expression
				//	write_expression();

				//	std::cout << t << std::endl;
				//}
				//else {
				//	//write_next_line(line);
				//}
				//break;
			}
			case TermType::OP: {
				//if (token == "=") continue;

				if (token == "{") {
					break;
				}

				if (token == "}") {
					return;
				}

				if (token[0] == '&') {
					std::string op = token;
					write_expression();
					if (op != "&amp;") {
						fo << op[1] << op[2] << std::endl;
					}
					else {
						fo << "and" << std::endl;
					}
					if (token == ")") {
						//outvar = 1;
						return;
					}
					break;
				}

				while (!operands.empty() && precedence(operands.top()) > precedence(token)) {
					if (operands.top() != "(") {
						fo << to_string_symbol(operands.top()) << std::endl;
						operands.pop();
					}
					else {
						break;
					}
				}

				operands.push(token);
				break;
			}
			case TermType::KEYWORD_CONSTANT: {
				write_keyword();
				break;
			}
			default:
				break;
		}
	}
}

void write_expression_var_index_identifier()
{

}


void Parser::write_keyword_let()
{
	std::streampos let_pos = fs.tellg();
	std::string let_line = line;

	bool var_index = false;

	//identifier
	to_next_token();
	{
		std::string new_token = get_new_token();
		if (new_token == "[") {
			var_index = true;
		}
	}

	//restore state
	fs.seekg(let_pos);
	line = let_line;
	extract_type_and_token(line);

	if (var_index) {
		write_expression();
		//fo << "starting equals " << token << std::endl;

		//fo << "add" << std::endl;

		//=
		to_next_token();

		write_expression();

		fo << "pop temp 0" << std::endl;
		fo << "pop pointer 1" << std::endl;
		fo << "push temp 0" << std::endl;
		fo << "pop that 0" << std::endl;

		//write_expression();
	}
	else {
		//identifier
		to_next_token();

		Info const& info = symbol_table.return_info(token);
		//=
		to_next_token();

		write_expression();

		fo << "pop " << SymbolTable::kind_to_string(info.kind) << " " << info.index << std::endl;
	}


}

void Parser::write_keyword_do()
{
	//identifier
	to_next_token();

	is_do = true;
	write_subroutine_call();
	is_do = false;

	fo << "pop temp 0" << std::endl;
}

void Parser::write_keyword_if()
{
	size_t yes = if_label_count;
	size_t no = ++if_label_count;
	if_label_count++;
	std::string prefix = name_without_extension + "ifL";

	//open paren
	to_next_token();

	is_conditional = true;
	write_expression();
	is_conditional = false;

	fo << "not" << std::endl;

	fo << "if-goto " << prefix << yes << std::endl;

	//open curly paren
	to_next_token();

	write_statements();

	//close curly paren
	if (token != "}") {
		to_next_token();
	}


	fo << "goto " << prefix << no << std::endl;

	fo << "label " << prefix << yes << std::endl;

	std::string new_token = get_new_token();
	if (new_token == "else") {
		write_statements();
	}

	fo << "label " << prefix << no << std::endl;

}

void Parser::write_keyword_else()
{

}

void Parser::write_keyword_while()
{
	size_t yes = while_label_count;
	size_t no = ++while_label_count;
	while_label_count++;
	std::string prefix = name_without_extension + "whL";

	//open paren
	to_next_token();

	fo << "label " << prefix << no << std::endl;

	is_conditional = true;
	write_expression();
	is_conditional = false;

	fo << "not" << std::endl;

	fo << "if-goto " << prefix << yes << std::endl;

	//open curly paren
	to_next_token();

	write_statements();

	//close curly paren
	if (token != "}") {
		to_next_token();
	}

	fo << "goto " << prefix << no << std::endl;

	fo << "label " << prefix << yes << std::endl;

	//fo << "while end" << std::endl;

}

void Parser::write_keyword_return()
{
	std::string new_token = get_new_token();
	if (new_token != ";" && new_token != "function" && new_token != "method") {
		write_expression();
	}
	fo << "return" << std::endl;
}

void Parser::write_keyword_true()
{
	fo << "push constant 0" << std::endl;
	fo << "not" << std::endl;
}

void Parser::write_keyword_false()
{
	fo << "push constant 0" << std::endl;
}

void Parser::write_keyword_null_type()
{
	fo << "push constant 0" << std::endl;
}

void Parser::write_keyword_this()
{
	fo << "push pointer 0" << std::endl;
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
	while (std::getline(fs, line)) {
		extract_type_and_token(line);
		if (token == "}") {
			break;
		}

		(this->*(write_functions_type[static_cast<int>(check_type())]))();
	}
}

void Parser::extract_type_and_token(std::string const& str)
{
	prev_token = token;
	if (str == "</tokens>") {
		return;
	}

	size_t token_pos = str.find('>');
	type = str.substr(1, token_pos - 1);

	token_pos += 2;

	token = str.substr(token_pos, str.find_last_of('<') - token_pos - 1);

	if (token == "4") {
		std::cout << "";
	}
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
	write_functions_keyword_type[static_cast<int>(KeywordType::STATIC)] = &Parser::write_keyword_static;
	write_functions_keyword_type[static_cast<int>(KeywordType::FIELD)] = &Parser::write_keyword_field;
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

		if (token == "-" && (prev_token == "(" || prev_token == "=" || prev_token == ",")) {
			return TermType::UNARY_OP;
		}

		if (token == "[") {
			//to_next_token();
			return TermType::EXPRESSION;
		}

		if (token[0] == '&') {
			return TermType::OP;
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