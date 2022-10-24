simulator: assembler.o 
	g++ -std=c++11 -o simulator assembler.o

assembler.o: assembler.cpp
	g++ -std=c++11 -c assembler.cpp

clean:
	fm -f core simulator assembler.o
