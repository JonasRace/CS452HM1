shell: myshell.c
	gcc myshell.c lex.yy.c -o shell -lfl
clear:
	clean
