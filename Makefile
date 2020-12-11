CPPFLAGS=-Wall -Wextra -std=c++17 -lstdc++ -O2

all: computer_example computer_tests

computer_example: computer_example.cc computer.h
	clang $(CPPFLAGS) -o $@ $<
computer_tests: computer_tests.cc computer.h
	clang $(CPPFLAGS) -o $@ $<

clean:
	rm -f computer_example computer_tests

