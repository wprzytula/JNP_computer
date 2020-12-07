//#include <cstdio>
#include <iostream>
//#include <array>

using namespace std;

//array<char, sizeof("Homo homini lupus")> a {"Homo homini lupus"};

template<typename ...Args>
void printer(Args&&... args) {
    (std::cout << ... << args) << '\n';
}

int main() {
//    printf("%s\n", a.begin());
    printer("Homo", ' ', "homini", ' ', "lupus.");
    return 0;
}
