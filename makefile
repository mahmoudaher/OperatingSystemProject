ALL: compile run

compile:
		mkdir -p bin
		gcc -o ./bin/Program Program.c

run:
		clear
		./bin/Program	
		
		#Ahmed bahaa Ahmed momtaz 2-B G221210569
		#Mahmoud Aldaher 2-C G221210588
		#Göktuğ Yüceer 1-A B221210058
		#Khalid almuanen 2-B G211210575