
all:  dinningPhil

dinningPhil: main.cpp Philosopher.o Fork.o 
		g++ -lm -Wall -lm -lncurses -o dinningPhil Philosopher.o Fork.o main.cpp -lncurses -pthread
	
Philosopher.o: Philosopher.cpp
	g++  -c Philosopher.cpp
	
Fork.o: Fork.cpp
	g++   -c Wall Fork.cpp
	
clean:
	rm -rf *o dinningPhil