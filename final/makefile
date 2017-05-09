# Note: you need to make sure that the indented lines below
# are made with TABS.
#
# To compile the program put foodPass.cpp, sem.cpp,
# sem.h, and this file: Makefile in a directory
# on one of the lab Macs.  Then type 
#         make
# at the shell prompt while 'in' the directory

foodPass: foodPass.o sem.o
	g++ -o foodPass foodPass.o sem.o  
foodPass.o: foodPass.cpp sem.h
	g++ -c foodPass.cpp
sem.o: sem.cpp sem.h
	g++ -c sem.cpp
