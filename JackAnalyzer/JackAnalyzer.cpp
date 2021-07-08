#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>

std::string extract_name(std::string const& f)
{
	int pos = f.find(".");
	return std::string(f.substr(0, pos));
}


struct KeyTokens
{
	std::vector<std::string> keywords;
	std::vector<std::string> symbols;

	void read_files(std::string filename)
	{
		std::ifstream file;
		file.open(filename);

		std::stringstream stream;
		std::string line;
		while (std::getline(file, line)) {
			stream << line;
		}

		if (filename[0] == 'k') {
			while (stream >> line) {
				keywords.push_back(line);
			}
		}
		else if (filename[0] == 's') {
			while (stream >> line) {
				symbols.push_back(line);
			}
		}

		file.close();
	}
};

class Tokenizer
{
	std::string filename;
	std::ifstream fs;
	std::ofstream fo;
	std::vector<std::string> tokens;
	KeyTokens key_tokens;

private:
	void open_files(std::string const& name)
	{
		filename = name;
		fs.open(filename);

		std::string out_filename = extract_name(filename) + "T.xml";
		fo.open(out_filename);
	}

	void close_files()
	{
		fs.close();
		fo.close();
	}

	bool is_symbol(char c) const
	{
		for (auto e : key_tokens.symbols) {
			if (e[0] == c) {
				return true;
			}
		}

		return false;
	}

	bool is_keyword(std::string const& key)
	{
		for (auto e : key_tokens.keywords) {
			if (e == key) {
				return true;
			}
		}

		return false;
	}

	bool is_string_constant(std::string const& key)
	{
		if (key[0] == '"') {
			return true;
		}
		else return false;
	}

	bool is_integer_constant(std::string const& key)
	{
		if (key[0] >= 48 && key[0] <= 57) {
			return true;
		}
		else return false;
	}

	inline void write_symbol(char c)
	{
		std::string s("");
		s += c;
		if (c == '<') {
			s = "&lt;";
		}
		else if (c == '>') {
			s = "&gt;";
		}
		else if (c == '&') {
			s = "&amp;";
		}

		fo << "<symbol> " << s << " </symbol>" << std::endl;
	}

	inline void write_keyword(std::string const& keyword)
	{
		fo << "<keyword> " << keyword << " </keyword>" << std::endl;
	}

	inline void write_integer(std::string const& integer)
	{
		fo << "<integerConstant> " << integer << " </integerConstant>" << std::endl;
	}

	inline void write_identifier(std::string const& identifier)
	{
		fo << "<identifier> " << identifier << " </identifier>" << std::endl;
	}

	inline void write_string(std::string const& str)
	{
		fo << "<stringConstant> " << str << " </stringConstant>" << std::endl;
	}

	void write_token(std::string const& token)
	{
	}

public:
	Tokenizer(std::string filename)
	{
		open_files(filename);
		key_tokens.read_files("keywords.token");
		key_tokens.read_files("symbols.token");
	}

	void set_new_file(std::string name)
	{
		filename = name;
		close_files();
		open_files(name);
	}

	void tokenize()
	{
		std::cout << "Tokenizing " << filename << std::endl;
		fo << "<tokens>" << std::endl;
		std::stringstream line_stream;
		std::string line;

		bool block_comment = false;
		while (std::getline(fs, line)) {
			if (block_comment) {
				size_t pos = line.find("*/");
				if (pos != std::string::npos) {
					block_comment = false;
					line = line.substr(pos + 2);
				}
				continue;
			}

			size_t pos = line.find("//");
			if (pos != std::string::npos) {
				line = line.substr(0, pos);
			}

			pos = line.find("/*");
			if (pos != std::string::npos) {
				size_t end_pos = line.find("*/");
				if (end_pos == std::string::npos) {
					block_comment = true;
				}
				line = line.substr(0, pos);
			}

			line_stream << line;
		}

		size_t pos = 0;
		std::string l;
		std::string token;
		while (line_stream >> l) {
			size_t s = l.size();
			for (int i = 0; i < s; i++) {
				token = l.substr(0, i);

				if (l[i] == '\"') {
					char c;
					std::string temp;

					l = l.substr(i + 1);
					size_t pos = l.find('\"');
					if (pos != std::string::npos) {
						temp += "\"";
						temp += l.substr(0, pos + 1);
						tokens.push_back(temp);

						if (pos == l.size() - 1) {
							continue;
						}

						l = l.substr(pos + 1);
						s = l.size();
						i = 0;
					}
					else {
						temp += "\"" + l;
						while (line_stream.get(c)) {
							if (c == '\"') {
								break;
							}
							temp += c;
						}
						temp += "\"";
						tokens.push_back(temp);
						break;
					}
				}

				if (is_symbol(l[i])) {
					if (token.size() > 0) {
						tokens.push_back(token);
					}
					tokens.push_back(std::string("") + l[i]);
					l = l.substr(i + 1);
					s = l.size();
					i = -1;
					continue;
				}

				if (i == s - 1) {
					token = l.substr(0, i + 1);
					tokens.push_back(token);
				}
			}
		}

		for (auto& t : tokens) {
			if (is_symbol(t[0])) {
				write_symbol(t[0]);
			}
			else if (is_keyword(t)) {
				write_keyword(t);
			}
			else if (is_string_constant(t)) {
				t = t.substr(1, t.size() - 1 - 1);
				write_string(t);
			}
			else if (is_integer_constant(t)) {
				write_integer(t);
			}

			else write_identifier(t);
			//std::cout << t << std::endl;
		}

		fo << "</tokens>" << std::endl;
	}

	~Tokenizer()
	{
		close_files();
	}
};

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
	void open_files(std::string const& name)
	{
		filename = name;
		fs.open(filename);

		std::string n = extract_name(filename);
		n = n.substr(0, n.size() - 1);
		//std::cout << n << std::endl;
		fo.open(n + ".xml");
	}

	void close_files()
	{
		fs.close();
		fo.close();
	}

	void inc_indent()
	{
		indentation += "  ";
	}

	void dec_indent()
	{
		indentation = indentation.substr(0, indentation.size() - 2);
	}

	void indent()
	{
		fo << indentation;
	}

	std::string get_new_token()
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

	void write_line()
	{
		indent();
		fo << "<" << type << ">" << " " << token << " " << "</" << type << ">" << std::endl;
	}

	void write_next_line(std::string& str)
	{
		std::getline(fs, str);
		extract_type_and_token(str);
		write_line();
	}


	void write_keyword_class()
	{
		write_line();
	}

	void write_keyword_method()
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

	void write_keyword_function()
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

	void write_keyword_constructor()
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

	void write_keyword_class_var_dec()
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

	void write_keyword_int()
	{
		write_line();
	}

	void write_keyword_boolean()
	{
		write_line();
	}

	void write_keyword_char()
	{
		write_line();
	}

	void write_keyword_void()
	{
		write_line();
	}

	void write_keyword_var()
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

	void write_keyword_static()
	{
		write_line();
	}

	void write_keyword_field()
	{
		write_line();
	}

	void write_var_index()
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

	void write_subroutine_call()
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
			else write_next_line(line);  //empty brackets case
		}
	}

	void write_expression_list()
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

	void write_expression()
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
					break;
				}
				default:
					break;
			}
		}

		dec_indent();
		indent();
		fo << "</expression>" << std::endl;
	}


	void write_keyword_let()
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

	void write_keyword_do()
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

	void write_keyword_if()
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

	void write_keyword_else()
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

	void write_keyword_while()
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

	void write_keyword_return()
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

	void write_keyword_true()
	{
		write_line();
	}

	void write_keyword_false()
	{
		write_line();
	}

	void write_keyword_null_type()
	{
		write_line();
	}

	void write_keyword_this()
	{
		write_line();
	}

	void write_keyword()
	{
		(this->*(write_functions_keyword_type[static_cast<int>(check_keyword_type())]))();
	}

	void write_less_than()
	{
		indent();
		fo << "<symbol> &lt; </symbol>" << std::endl;
	}

	void write_greater_than()
	{
		indent();
		fo << "<symbol> &gt; </symbol>" << std::endl;
	}

	void write_equals()
	{
		indent();
		fo << "<symbol> &eq; </symbol>" << std::endl;
	}

	void write_symbol()
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

	void write_int_const()
	{
		write_line();
	}

	void write_string_const()
	{
		write_line();
	}

	void write_identifier()
	{
		write_line();
	}

	void write_statements()
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

	void extract_type_and_token(std::string const& str)
	{
		if (str == "</tokens>") {
			return;
		}

		size_t token_pos = str.find('>');
		type = str.substr(1, token_pos - 1);
		//std::cout << type << std::endl;

		token_pos += 2;

		token = str.substr(token_pos, str.find_last_of('<') - token_pos - 1);

		//std::cout << "token: " << token << std::endl;
	}

	void write_functions_vector_init()
	{
		write_functions_type.resize(static_cast<int>(TokenType::NUM_OF_ITEMS_IN_ENUM));
		write_functions_type[static_cast<int>(TokenType::KEYWORD)] = &Parser::write_keyword;
		write_functions_type[static_cast<int>(TokenType::SYMBOL)] = &Parser::write_symbol;
		write_functions_type[static_cast<int>(TokenType::INTEGER_CONSTANT)] = &Parser::write_int_const;
		write_functions_type[static_cast<int>(TokenType::STRING_CONSTANT)] = &Parser::write_string_const;
		write_functions_type[static_cast<int>(TokenType::IDENTIFIER)] = &Parser::write_identifier;
	}

	void write_functions_keyword_type_vector_init()
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

	TokenType check_type()
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

	KeywordType check_keyword_type()
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

	TermType check_term_type()
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

	bool is_statement()
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

public:
	Parser(std::string const& filename)
	{
		open_files(filename);
		write_functions_vector_init();
		write_functions_keyword_type_vector_init();
	}

	void parse()
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

	~Parser()
	{
		close_files();
	}
};

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