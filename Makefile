# Makefile

objects = main.o

edit: $(objects)
	g++ -o -std=C++11 [main main.cpp] $(objects)

main.cpp: main.cpp
	g++ -c -std=C++11 main.cpp
	
    
	
