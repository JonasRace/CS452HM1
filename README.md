
# CS452HM1
CS452 Lab 1 - C Shell Implementation

Authors: Jonas Eckert and Yizheng Chong

-----------------------------------------------------------------

***Contents***
lex.c - lex.c provides a function to get and parse a line of input. IT returns an array of pointers to character strings.

lex.yy.c - This file is produced when lex.c is compiled using flex: "flex lex.c". 

myshell.c - This is the main file. myshell.c contains the code that implements the shell. This shell implementations includes functionality such as appending, background processes, logical operators, command separators, and pipes. It also implements basic shell functions such as commands with and without arguments and input redirection.

makefile - This is the makefile that compiles all the necessary files and outputs a file that is used to run the program.

***How to Run***
To compile - In your terminal, navigate to the folder where all of the above files are located. Then, type "make". This will run the makefile which runs the following command:
		
		gcc myshell.c lex.yy.c -o shell -lfl

After the makefile is executed, the shell program can be run using the following command:

		./shell

	

	
