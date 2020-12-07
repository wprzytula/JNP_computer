#include <iostream>
#include <map>

using namespace std;

constexpr map<int> mapWithOne() {
    map mapa;
    mapa.emplace(1, 1);
}

map mapa = mapWithOne();

int main() {
    map mapWithOne();
    return 0;
}
