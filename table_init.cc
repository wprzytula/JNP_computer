#include <iostream>
#include <array>

using namespace std;

constexpr int N = 5;

constexpr int f(int x) {
  return x * 2;
}

using A = array<int, N>;

template<int... i>
constexpr A fs() {
  return A{{f(i)...}};
}

template<int...>
struct S;

template<int... i>
struct S<0, i...> {
  static constexpr A gs() {
    return fs<0, i...>();
  }
};

template<int i, int... j>
struct S<i, j...> {
  static constexpr A gs() {
    return S<i - 1, i, j...>::gs();
  }
};

//constexpr auto X = S<N - 1>::gs();
constexpr auto X = S<4, 17>::gs();
//constexpr auto X = S<0>::gs();
//constexpr auto X = fs<3, 5>();

int main() {
  for (auto x : X)
    cout << x << endl;
}
