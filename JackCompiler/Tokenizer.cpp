#include "Tokenizer.hpp"

void KeyTokens::read_files(std::string filename)
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

void Tokenizer::open_files(std::string const& name)
{
	filename = name;
	fs.open(filename);

	std::string out_filename = extract_name(filename) + "T.xml";
	fo.open(out_filename);
}

void Tokenizer::close_files()
{
	fs.close();
	fo.close();
}

bool Tokenizer::is_symbol(char c) const
{
	for (auto e : key_tokens.symbols) {
		if (e[0] == c) {
			return true;
		}
	}

	return false;
}

bool Tokenizer::is_keyword(std::string const& key)
{
	for (auto e : key_tokens.keywords) {
		if (e == key) {
			return true;
		}
	}

	return false;
}

bool Tokenizer::is_string_constant(std::string const& key)
{
	if (key[0] == '"') {
		return true;
	}
	else return false;
}

bool Tokenizer::is_integer_constant(std::string const& key)
{
	if (key[0] >= 48 && key[0] <= 57) {
		return true;
	}
	else return false;
}

inline void Tokenizer::write_symbol(char c)
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

inline void Tokenizer::write_keyword(std::string const& keyword)
{
	fo << "<keyword> " << keyword << " </keyword>" << std::endl;
}

inline void Tokenizer::write_integer(std::string const& integer)
{
	fo << "<integerConstant> " << integer << " </integerConstant>" << std::endl;
}

inline void Tokenizer::write_identifier(std::string const& identifier)
{
	fo << "<identifier> " << identifier << " </identifier>" << std::endl;
}

inline void Tokenizer::write_string(std::string const& str)
{
	fo << "<stringConstant> " << str << " </stringConstant>" << std::endl;
}


Tokenizer::Tokenizer(std::string filename)
{
	open_files(filename);
	key_tokens.read_files("keywords.token");
	key_tokens.read_files("symbols.token");
}

void Tokenizer::set_new_file(std::string name)
{
	filename = name;
	close_files();
	open_files(name);
}

void Tokenizer::tokenize()
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

Tokenizer::~Tokenizer()
{
	close_files();
}