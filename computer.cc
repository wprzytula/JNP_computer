#include <climits>
#include "computer.h"
#include "iostream"

using mymap =  ct_map<kv<10, 20>, kv<11, 21>, kv<23, 7>>;

using mymap2 = mymap::add<1, 2>::result;

//int main() {
////    std::cout << Id("HELLO!") << std::endl << UINT_MAX;
//    std::cout << mymap2::get<1>::val;
//}