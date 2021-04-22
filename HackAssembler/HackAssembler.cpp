#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

struct Symbolic_Instruction
{
	std::string command;
	std::string opcode;

	Symbolic_Instruction() : command(""), opcode("") {}
	Symbolic_Instruction(std::string const& command, std::string const& opcode) :command(command), opcode(opcode) {}
	friend std::ostream& operator<<(std::ostream& os, Symbolic_Instruction& ins)
	{
		os << ins.command << " " << ins.opcode;
		return os;
	}
};

struct Lookup_Table
{
	const std::vector<Symbolic_Instruction> comp = read_ins_file("comp.ins");
	const std::vector<Symbolic_Instruction> dest = read_ins_file("dest.ins");
	const std::vector<Symbolic_Instruction> jump = read_ins_file("jump.ins");
	std::vector<Symbolic_Instruction> symbol_table = read_ins_file("pre_symbols.ins");

public:
	std::vector<Symbolic_Instruction> read_ins_file(std::string const& filename)
	{
		std::ifstream f;
		f.open(filename);

		std::vector<Symbolic_Instruction> table;

		std::string line;
		while (!f.eof()) {
			std::getline(f, line);
			int pos = line.find(",");

			std::string command = line.substr(0, pos);
			std::string opcode = line.substr(pos + 1);

			Symbolic_Instruction ins(command, opcode);

			table.push_back(ins);
		}

		f.close();

		return table;
	}
};


struct C_Ins
{
	std::string lhs;
	std::string rhs;

public:
	C_Ins() : lhs(""), rhs("") {}
	C_Ins(std::string const& lhs, std::string const& rhs) : lhs(lhs), rhs(rhs) {}
	friend std::ostream& operator<<(std::ostream& os, C_Ins& ins)
	{
		os << ins.lhs << " " << ins.rhs;
		return os;
	}
};


std::string extract_name(std::string const& f)
{
	int pos = f.find(".");
	return std::string(f.substr(0, pos));
}


std::string a_to_machine_code(std::string const& ins)
{
	std::string machine_code("0");

	int value = stoi(ins.substr(1));

	std::string bin_value;

	if (!value) {
		machine_code += "000000000000000";
		return machine_code;
	}

	while (value > 0) {
		bin_value += std::to_string(value % 2);
		value /= 2;
	}

	std::reverse(bin_value.begin(), bin_value.end());

	int bin_val_size = bin_value.size();
	int num_zeroes_to_add = 15 - bin_val_size;

	for (int i = 0; i < num_zeroes_to_add; i++) {
		machine_code += "0";
	}

	machine_code += bin_value;

	return machine_code;
}


C_Ins break_instruction(std::string const& ins)
{
	int temp = 0;
	int pos = 0;
	C_Ins c_ins;

	if (ins.find("=") != std::string::npos) {
		pos = ins.find("=");
	}
	else if (ins.find(";") != std::string::npos) {
		pos = ins.find(";");
	}

	c_ins.lhs = ins.substr(0, pos);
	c_ins.rhs = ins.substr(pos + 1);

	return c_ins;
}


std::string c_to_machine_code(std::string const& ins, Lookup_Table const& table)
{
	std::string machine_code = "111";

	C_Ins c_ins = break_instruction(ins);

	bool dest_ins = true;
	if (ins.find("=") == std::string::npos) {
		dest_ins = false;
	}

	if (dest_ins) {
		for (Symbolic_Instruction const& instruction : table.comp) {
			if (c_ins.rhs == instruction.command) {
				machine_code += instruction.opcode;
			}
		}

		for (Symbolic_Instruction const& instruction : table.dest) {
			if (c_ins.lhs == instruction.command) {
				machine_code += instruction.opcode;
			}
		}
		machine_code += "000";
	}
	else {
		for (Symbolic_Instruction const& instruction : table.comp) {
			if (c_ins.lhs == instruction.command) {
				machine_code += instruction.opcode;
			}
		}
		machine_code += "000";
		for (Symbolic_Instruction const& instruction : table.jump) {
			if (c_ins.rhs == instruction.command) {
				machine_code += instruction.opcode;
			}
		}
	}

	return machine_code;
}


std::string convert_to_machine_code(std::string const& ins, Lookup_Table const& table)
{
	std::string machine_code;

	if (ins[0] == '@') {
		machine_code = a_to_machine_code(ins);
		std::cout << machine_code << std::endl;
	}
	else {
		machine_code = c_to_machine_code(ins, table);
	}

	std::cout << "ins " << ins << std::endl;
	std::cout << machine_code << std::endl;

	return machine_code;
}


void first_pass(std::ifstream& f, std::vector<Symbolic_Instruction>& symbol_table)
{
	std::string ins;

	int count = 0;
	while (std::getline(f, ins)) {
		if ((ins[0] == '/' && ins[1] == '/') || ins[0] == '\n' || ins[0] == 0) continue;

		if (ins[0] == '(') {
			std::string label = ins.substr(1, ins.size() - 2);
			symbol_table.push_back(Symbolic_Instruction(label, std::to_string(count)));
			continue;
		}
		count++;
	}

	f.clear();
	f.seekg(0);
}


void second_pass(std::ifstream& f, std::vector<Symbolic_Instruction>& symbol_table)
{
	std::string ins;
	int address = 16;

	std::ofstream temp_f;
	temp_f.open("temp.hack");

	while (std::getline(f, ins)) {
		ins.erase(std::remove_if(ins.begin(), ins.end(), isspace), ins.end());
		if ((ins[0] == '/' && ins[1] == '/') || ins[0] == '\n' || ins[0] == 0 || ins[0] == '(') continue;
		bool is_A_ins = false;

		if (ins[0] == '@') {
			is_A_ins = true;
			std::string value = ins.substr(1);
			value.erase(std::remove_if(value.begin(), value.end(), isspace), value.end());

			if (value[0] >= '0' && value[0] <= '9') {
				temp_f << "@" << value << std::endl;
				continue;
			}

			bool added = false;
			for (auto sym : symbol_table) {
				if (sym.command == value) {
					temp_f << "@" << sym.opcode << std::endl;
					added = true;
					break;
				}
			}

			if (!added) {
				symbol_table.push_back(Symbolic_Instruction(value, std::to_string(address)));
				temp_f << "@" << address << std::endl;
				address++;
				continue;
			}
		}

		if (!is_A_ins) {
			temp_f << ins << std::endl;
		}
	}

	temp_f.close();
}


void third_pass(char* argv[], Lookup_Table& table)
{
	std::ifstream file;
	file.open("temp.hack");

	std::ofstream fout;
	fout.open(extract_name(argv[1]) + ".hack");

	std::string ins;

	while (std::getline(file, ins)) {
		std::string machine_code = convert_to_machine_code(ins, table);

		fout << machine_code << std::endl;
	}

	fout.close();
}


int main(int argc, char* argv[])
{
	Lookup_Table table;

	std::ifstream f;
	f.open(argv[1]);

	first_pass(f, table.symbol_table);

	second_pass(f, table.symbol_table);

	f.close();

	third_pass(argv, table);
}