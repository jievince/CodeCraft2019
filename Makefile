# Makefile

objects = main.o

edit: $(objects)
	g++ -o [main main.cpp] $(objects)

main.cpp: main.cpp
	g++ -c main.cpp
	
    
	
