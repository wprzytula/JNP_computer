CPPFLAGS=-Wall -Wextra -std=c++17 -lstdc++ -O2

all: computer_example

computer.o: computer.cc computer.h
	clang $(CPPFLAGS) -c -o $@ $< 
computer_example.o: computer_example.cc computer.h
	clang $(CPPFLAGS) -c -o $@ $<
computer_example: computer_example.o computer.o
	clang $(CPPFLAGS) -o $@ $^

clean:
	rm -f *.o computer_example

