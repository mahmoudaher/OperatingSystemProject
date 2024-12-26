ALL: compile run

compile:
		mkdir -p bin
		gcc -o ./bin/Program Program.c

run:
		clear
		./bin/Program	