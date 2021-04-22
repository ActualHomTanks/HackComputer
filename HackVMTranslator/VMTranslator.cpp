#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>


struct Commands
{
	std::vector<std::string> arithmetic;
	std::vector<std::string> mem_access;
	std::vector<std::string> branch;
	std::vector<std::string> function;

public:
	Commands()
	{
		arithmetic = get_commands("arithmetic.ins");
		mem_access = get_commands("mem_access.ins");
		branch = get_commands("branch.ins");
		function = get_commands("function.ins");
	}

	std::vector<std::string> get_commands(const std::string filename)
	{
		std::ifstream command_file;
		command_file.open(filename);

		std::vector<std::string> commands;

		std::string command;
		while (getline(command_file, command)) {
			commands.push_back(command);
		}

		command_file.close();

		return commands;
	}
};


class Parser
{
	std::ifstream& f;
	std::string cur_ins;
public:
	Parser(std::ifstream& f) : f(f) {}

	bool advance_cur_instruction()
	{
		while (std::getline(f, cur_ins)) {
			if ((cur_ins[0] == '/' && cur_ins[1] == '/') || cur_ins[0] == 0 || cur_ins[0] == '\n') continue;
			return true;
		}
		return false;
	}

	std::vector<std::string> parse_cur_instruction()
	{
		std::stringstream words(cur_ins);
		std::string word;
		std::vector<std::string> broken_ins;

		while (words >> word) {
			if (word[0] == '/' && word[1] == '/') break;
			broken_ins.push_back(word);
		}

		return broken_ins;
	}
};


enum class COMMAND_TYPE
{
	C_ARITHMETIC,
	C_PUSH,
	C_POP,
	C_LABEL,
	C_GOTO,
	C_IF,
	C_FUNCTION,
	C_RETURN,
	C_CALL
};


class Code_Writer
{
public:
	std::ofstream& fout;
	std::string filename;
	std::vector<std::string> broken_ins;
	unsigned long label_count = 0;

private:
	void generic_push()
	{
		fout << "D=M" << std::endl;
		fout << "@" << broken_ins[2] << std::endl;
		fout << "A=D+A" << std::endl;
		fout << "D=M" << std::endl;
		generic_stack_push();
	}

	//pushes value in D register to stack
	void generic_stack_push()
	{
		fout << "@SP" << std::endl;
		fout << "A=M" << std::endl;
		fout << "M=D" << std::endl;
		fout << "@SP" << std::endl;
		fout << "M=M+1" << std::endl;
	}

	void generic_pop()
	{
		fout << "D=M" << std::endl;
		fout << "@" << broken_ins[2] << std::endl;
		fout << "D=D+A" << std::endl;
		fout << "@R13" << std::endl;
		fout << "M=D" << std::endl;
		generic_stack_pop();
		fout << "@R13" << std::endl;
		fout << "A=M" << std::endl;
		fout << "M=D" << std::endl;
	}

	//puts popped value in D register
	void generic_stack_pop()
	{
		fout << "@SP" << std::endl;
		fout << "AM=M-1" << std::endl;
		fout << "D=M" << std::endl;
	}

	void generic_stack_pop_into_A()
	{
		fout << "@SP" << std::endl;
		fout << "AM=M-1" << std::endl;
		fout << "A=M" << std::endl;
	}

public:
	Code_Writer(std::ofstream& fout, std::string const& filename) : fout(fout), filename(filename) {}

	void update_instruction(std::vector<std::string>& new_broken_ins)
	{
		broken_ins = new_broken_ins;
	}

	void check_new_file()
	{
		if (broken_ins[0] == "STATIC") {
			filename = broken_ins[2];
		}
	}

	COMMAND_TYPE get_command_type(std::string const& ins)
	{
		//std::cout << "ins: " << ins << std::endl;
		if (ins == "push") {
			return COMMAND_TYPE::C_PUSH;
		}
		else if (ins == "pop") {
			return COMMAND_TYPE::C_POP;
		}
		else if (ins == "add" || ins == "sub" || ins == "neg" || ins == "eq" || ins == "gt" || ins == "lt" || ins == "and" ||
			ins == "or" || ins == "not") {
			return COMMAND_TYPE::C_ARITHMETIC;
		}
		else if (ins == "label") {
			return COMMAND_TYPE::C_LABEL;
		}
		else if (ins == "goto") {
			return COMMAND_TYPE::C_GOTO;
		}
		else if (ins == "if-goto") {
			return COMMAND_TYPE::C_IF;
		}
		else if (ins == "function") {
			return COMMAND_TYPE::C_FUNCTION;
		}
		else if (ins == "call") {
			return COMMAND_TYPE::C_CALL;
		}
		else if (ins == "return") {
			return COMMAND_TYPE::C_RETURN;
		}
	}

	void write_init()
	{
		fout << "@256" << std::endl;
		fout << "D=A" << std::endl;
		fout << "@SP" << std::endl;
		fout << "M=D" << std::endl;

		std::vector<std::string> v = { "call", "Sys.init", "0" };
		update_instruction(v);
		write_assembly_for_vm_instruction();
	}

	void write_assembly_for_vm_instruction()
	{
		fout << "// ";
		for (std::string const& word : broken_ins) {
			fout << word << ' ';
		}
		fout << '\n';

		COMMAND_TYPE command_type = get_command_type(broken_ins[0]);

		switch (command_type) {
			case COMMAND_TYPE::C_PUSH: {
				write_push();
				break;
			}
			case COMMAND_TYPE::C_POP: {
				write_pop();
				break;
			}
			case COMMAND_TYPE::C_ARITHMETIC: {
				write_arithmetic();
				break;
			}
			case COMMAND_TYPE::C_LABEL: {
				write_label();
				break;
			}
			case COMMAND_TYPE::C_GOTO: {
				write_goto();
				break;
			}
			case COMMAND_TYPE::C_IF: {
				write_if_goto();
				break;
			}
			case COMMAND_TYPE::C_FUNCTION: {
				write_function();
				break;
			}
			case COMMAND_TYPE::C_CALL: {
				write_call();
				break;
			}
			case COMMAND_TYPE::C_RETURN: {
				write_return();
				break;
			}

		}
	}

	void write_arithmetic()
	{
		if (broken_ins[0] == "add") {
			write_add();
		}
		else if (broken_ins[0] == "sub") {
			write_sub();
		}
		else if (broken_ins[0] == "neg") {
			write_neg();
		}
		else if (broken_ins[0] == "eq") {
			write_eq();
		}
		else if (broken_ins[0] == "gt") {
			write_gt();
		}
		else if (broken_ins[0] == "lt") {
			write_lt();
		}
		else if (broken_ins[0] == "and") {
			write_and();
		}
		else if (broken_ins[0] == "or") {
			write_or();
		}
		else if (broken_ins[0] == "not") {
			write_not();
		}
	}

	void write_push()
	{
		if (broken_ins[1] == "constant") {
			fout << "@" << broken_ins[2] << std::endl;
			fout << "D=A" << std::endl;
			generic_stack_push();
		}
		else if (broken_ins[1] == "local") {
			fout << "@LCL" << std::endl;
			generic_push();
		}
		else if (broken_ins[1] == "argument") {
			fout << "@ARG" << std::endl;
			generic_push();
		}
		else if (broken_ins[1] == "this") {
			fout << "@THIS" << std::endl;
			generic_push();
		}
		else if (broken_ins[1] == "that") {
			fout << "@THAT" << std::endl;
			generic_push();
		}
		else if (broken_ins[1] == "temp") {
			fout << "@5" << std::endl;
			fout << "D=A" << std::endl;
			fout << "@" << broken_ins[2] << std::endl;
			fout << "A=D+A" << std::endl;
			fout << "D=M" << std::endl;
			generic_stack_push();
		}
		else if (broken_ins[1] == "static") {
			fout << "@" << filename << "." << broken_ins[2] << std::endl;
			fout << "D=M" << std::endl;
			generic_stack_push();
		}
		else if (broken_ins[1] == "pointer") {
			if (broken_ins[2] == "0") {
				fout << "@THIS" << std::endl;
				fout << "D=M" << std::endl;
				generic_stack_push();
			}
			else if (broken_ins[2] == "1") {
				fout << "@THAT" << std::endl;
				fout << "D=M" << std::endl;
				generic_stack_push();
			}
		}
	}

	void write_pop()
	{
		if (broken_ins[1] == "local") {
			fout << "@LCL" << std::endl;
			generic_pop();
		}
		else if (broken_ins[1] == "argument") {
			fout << "@ARG" << std::endl;
			generic_pop();
		}
		else if (broken_ins[1] == "this") {
			fout << "@THIS" << std::endl;
			generic_pop();
		}
		else if (broken_ins[1] == "that") {
			fout << "@THAT" << std::endl;
			generic_pop();
		}
		else if (broken_ins[1] == "temp") {
			fout << "@5" << std::endl;
			fout << "D=A" << std::endl;
			fout << "@" << broken_ins[2] << std::endl;
			fout << "D=D+A" << std::endl;
			fout << "@R13" << std::endl;
			fout << "M=D" << std::endl;
			generic_stack_pop();
			fout << "@R13" << std::endl;
			fout << "A=M" << std::endl;
			fout << "M=D" << std::endl;
		}
		else if (broken_ins[1] == "static") {
			generic_stack_pop();
			fout << "@" << filename << "." << broken_ins[2] << std::endl;
			fout << "M=D" << std::endl;
		}
		else if (broken_ins[1] == "pointer") {
			if (broken_ins[2] == "0") {
				fout << "@THIS" << std::endl;
				fout << "D=A" << std::endl;
				fout << "@R13" << std::endl;
				fout << "M=D" << std::endl;
				generic_stack_pop();
				fout << "@R13" << std::endl;
				fout << "A=M" << std::endl;
				fout << "M=D" << std::endl;
			}
			else if (broken_ins[2] == "1") {
				fout << "@THAT" << std::endl;
				fout << "D=A" << std::endl;
				fout << "@R13" << std::endl;
				fout << "M=D" << std::endl;
				generic_stack_pop();
				fout << "@R13" << std::endl;
				fout << "A=M" << std::endl;
				fout << "M=D" << std::endl;
			}
		}
	}

	void write_add()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "M=M+D" << std::endl;
	}

	void write_sub()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "M=M-D" << std::endl;
	}

	void write_neg()
	{
		fout << "@SP" << std::endl;
		fout << "A=M-1" << std::endl;
		fout << "M=-M" << std::endl;
	}

	void write_and()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "M=D&M" << std::endl;
	}

	void write_or()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "M=D|M" << std::endl;
	}

	void write_not()
	{
		fout << "@SP" << std::endl;
		fout << "A=M-1" << std::endl;
		fout << "M=!M" << std::endl;
	}

	void write_eq()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "D=M-D" << std::endl;
		fout << "M=-1" << std::endl;
		label_count++;
		std::string label = "eqtrue" + std::to_string(label_count);
		fout << "@" << label << std::endl;
		fout << "D;JEQ" << std::endl;
		fout << "@SP" << std::endl;
		fout << "A=M-1" << std::endl;
		fout << "M=0" << std::endl;
		fout << "(" << label << ")" << std::endl;
	}

	void write_gt()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "D=M-D" << std::endl;
		fout << "M=-1" << std::endl;
		label_count++;
		std::string label = "gttrue" + std::to_string(label_count);
		fout << "@" << label << std::endl;
		fout << "D;JGT" << std::endl;
		fout << "@SP" << std::endl;
		fout << "A=M-1" << std::endl;
		fout << "M=0" << std::endl;
		fout << "(" << label << ")" << std::endl;
	}

	void write_lt()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "D=M-D" << std::endl;
		fout << "M=-1" << std::endl;
		label_count++;
		std::string label = "lttrue" + std::to_string(label_count);
		fout << "@" << label << std::endl;
		fout << "D;JLT" << std::endl;
		fout << "@SP" << std::endl;
		fout << "A=M-1" << std::endl;
		fout << "M=0" << std::endl;
		fout << "(" << label << ")" << std::endl;
	}

	void write_label()
	{
		fout << "(" << broken_ins[1] << ")" << std::endl;
	}

	void write_goto()
	{
		fout << "@" << broken_ins[1] << std::endl;
		fout << "0;JMP" << std::endl;
	}

	void write_if_goto()
	{
		generic_stack_pop();
		fout << "A=A-1" << std::endl;
		fout << "@" << broken_ins[1] << std::endl;
		fout << "D;JNE" << std::endl;
	}

	void write_push_constant_zero()
	{
		fout << "@0" << std::endl;
		fout << "D=A" << std::endl;
		fout << "@SP" << std::endl;
		fout << "A=M" << std::endl;
		fout << "M=D" << std::endl;
		fout << "@SP" << std::endl;
		fout << "M=M+1" << std::endl;
	}

	void write_function()
	{
		fout << "(" << broken_ins[1] << ")" << std::endl;
		for (int i = 0; i < std::stoi(broken_ins[2]); i++) {
			write_push_constant_zero();
		}
	}

	void write_call()
	{
		fout << "@" << broken_ins[1] << "$ret" << ++label_count << std::endl;
		fout << "D=A" << std::endl;
		generic_stack_push();
		fout << "@LCL" << std::endl;
		fout << "D=M" << std::endl;
		generic_stack_push();
		fout << "@ARG" << std::endl;
		fout << "D=M" << std::endl;
		generic_stack_push();
		fout << "@THIS" << std::endl;
		fout << "D=M" << std::endl;
		generic_stack_push();
		fout << "@THAT" << std::endl;
		fout << "D=M" << std::endl;
		generic_stack_push();
		fout << "@SP" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@5" << std::endl;
		fout << "D=D-A" << std::endl;
		fout << "@" << broken_ins[2] << std::endl;
		fout << "D=D-A" << std::endl;
		fout << "@ARG" << std::endl;
		fout << "M=D" << std::endl;
		fout << "@SP" << std::endl;
		//fout << "D=A" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@LCL" << std::endl;
		fout << "M=D" << std::endl;
		fout << "@" << broken_ins[1] << std::endl;
		fout << "0;JMP" << std::endl;
		fout << "(" << broken_ins[1] << "$ret" << label_count << ")" << std::endl;
	}

	void write_pre_frame_return_template(std::string s)
	{
		fout << "@R11" << std::endl;
		fout << "D=M-1" << std::endl;
		fout << "AM=D" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@" << s << std::endl;
		fout << "M=D" << std::endl;;
	}

	void write_return()
	{
		fout << "@LCL" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@R11" << std::endl; // endframe
		fout << "M=D" << std::endl;
		fout << "@5" << std::endl;
		fout << "A=D-A" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@R12" << std::endl; //retAddr
		fout << "M=D" << std::endl;
		fout << "@ARG" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@" << 0 << std::endl;
		fout << "D=D+A" << std::endl;
		fout << "@R13" << std::endl;
		fout << "M=D" << std::endl;
		generic_stack_pop();
		fout << "@R13" << std::endl;
		fout << "A=M" << std::endl;
		fout << "M=D" << std::endl;
		fout << "@ARG" << std::endl;
		fout << "D=M" << std::endl;
		fout << "@SP" << std::endl;
		fout << "M=D+1" << std::endl;
		write_pre_frame_return_template("THAT");
		write_pre_frame_return_template("THIS");
		write_pre_frame_return_template("ARG");
		write_pre_frame_return_template("LCL");
		fout << "@R12" << std::endl;
		fout << "A=M" << std::endl;
		fout << "0;JMP" << std::endl;
	}
};


std::string extract_name(std::string const& f)
{
	int pos = f.find(".");
	return std::string(f.substr(0,pos));
}

std::string extract_name_for_static(std::string const& f)
{
	unsigned int first_pos = f.find_last_of("/");

	first_pos = first_pos + 1;

	return std::string(f.substr(first_pos, f.size() - first_pos));
}


std::string extract_name_for_static_windows(std::string const& f)
{
	unsigned int first_pos = f.find_last_of("\\");

	first_pos = first_pos + 1;

	return std::string(f.substr(first_pos, f.size() - first_pos));
}


bool is_directory(std::string s)
{
	if (s.find(".vm") == std::string::npos) {
		return true;
	}
	else return false;
}


int main(int argc, char* argv[])
{
	Commands commands;

	if (is_directory(argv[1])) {
		std::string path(std::filesystem::current_path().string());
		path += "\\" + std::string(argv[1]);
		std::cout << path << std::endl;

		std::ofstream new_vm;
		new_vm.open(extract_name(argv[1]) + ".vm");

		for (auto f : std::filesystem::directory_iterator(path)) {			
			std::ifstream file;
			file.open(f.path().string());

			new_vm << "STATIC NAME: " << extract_name_for_static_windows(f.path().string()) << std::endl;
			
			std::string line;
			while (std::getline(file, line)) {
				new_vm << line << std::endl;
			}

			file.close();
		}

		new_vm.close();

		std::ifstream f;
		std::string input_name(argv[1]);
		input_name += ".vm";
		f.open(std::string(input_name));

		Parser parser(f);

		std::ofstream fout;
		std::string fout_name = extract_name(argv[1]);
		fout.open(fout_name + ".asm");
		Code_Writer code_writer(fout, extract_name_for_static(fout_name));
		code_writer.write_init();

		std::vector<std::string> broken_ins;

		while (parser.advance_cur_instruction()) {
			broken_ins = parser.parse_cur_instruction();

			for (auto ins : broken_ins) {
				std::cout << ins << ' ';
			}

			std::cout << std::endl;

			code_writer.update_instruction(broken_ins);
			code_writer.check_new_file();
			code_writer.write_assembly_for_vm_instruction();
		}

		f.close();
		fout.close();
	}
	else {
		std::ifstream f;
		f.open(argv[1]);

		Parser parser(f);

		std::ofstream fout;
		std::string fout_name = extract_name(argv[1]);
		fout.open(fout_name + ".asm");
		Code_Writer code_writer(fout, extract_name_for_static(fout_name));
		//code_writer.write_init();

		std::vector<std::string> broken_ins;

		while (parser.advance_cur_instruction()) {
			broken_ins = parser.parse_cur_instruction();

			for (auto ins : broken_ins) {
				std::cout << ins << ' ';
			}

			std::cout << std::endl;

			code_writer.update_instruction(broken_ins);
			code_writer.write_assembly_for_vm_instruction();
		}

		f.close();
		fout.close();
	}
	
}