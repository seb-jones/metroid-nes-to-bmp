a.out: code/*.c
	gcc -g -O0 code/main.c -lm
	ctags -R
