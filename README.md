# Shell Programming

### Contents:

	- shell.h: definition of the command structure, and prototypes for the provided functions parse_command and free_command .
	
	- parser.c: implementation of parse_command and free_command .
	
	- tests/: a subdirectory containing test inputs, expected outputs, and helper commands. 
	
	- run-tests: a sh script to run the tests and compare your shell's output with the expected output.

### Running:
	
	Use the makefile provided to compile the program.
	
	Run ./run-tests to test all 25 cases.

### Implementation notes:

	- readcmd() to read and parse one line of the command at a time

	- call_redirected() is used for the external command executions
	
	- A command is recognized as built-in if it meets one the if/else if statements within main(), other commands are external.