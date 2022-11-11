# HackComputer
A WIP repo of the HackComputer Toolchain as part of nand2tetris

All of the following tools are written in C++

**HackAssembler:** Converts the Hack Assembly Language to Hack Machine Code.

**HackVMTranslator:** Converts the HackVM code into Hack Assembly.

**JackCompiler:** Converts the high level Jack Programming Language into HackVM code.
- Currently tokenizes and parses .jack files and outputs a parse tree in xml form.
- Code generation afterr parsing into vm code.

**JackOS:** Operating System for the Hack Computer written using the Jack Programming Language.
