#include "computer.h"
#include <array>
#include <iostream>

// Operator == dla std::array jest constexpr dopiero od C++20.
template<class T, std::size_t N>
constexpr bool compare(std::array<T, N> const &arg1, std::array<T, N> const &arg2) {
    for (size_t i = 0; i < N; ++i)
        if (arg1[i] != arg2[i]) return false;
    return true;
}

/*
 * Tests by Wojciech Przytuła
 * All rights left
 * */

using test_machine = Computer<4, int>;

// the following tests presume that you use unsigned long long for storing IDs


// tests that should compile and not raise static_assert errors

using test_empty = Program<>;
constexpr std::array<int, 4> test_empty_res = {0, 0, 0, 0};
static_assert(compare(test_machine::boot<test_empty>(), test_empty_res));

using test_jumps = Program<
    Dec<Mem<Num<0>>>,
    Jz<Id("XD")>,
    Inc<Mem<Num<0>>>,
    Jz<Id("XD")>,
    Inc<Mem<Num<1>>>,
    Label<Id("xd")>
>;
constexpr std::array<int, 4> test_jumps_res = {0, 0, 0, 0};
static_assert(compare(test_machine::boot<test_jumps>(), test_jumps_res));

using test_declarations = Program<
    D<Id("a"), Num<-3>>,
    D<Id("b"), Num<-2>>,
    D<Id("c"), Num<-1>>,
    D<Id("a"), Num<-1>>
>;
constexpr std::array<int, 4> test_declarations_res = {-3, -2, -1, -1};
static_assert(compare(test_machine::boot<test_declarations>(), test_declarations_res));


// tests that should compile AND ofc not result in infinite loop:
using test_id1 = Program<
    D<Id("a"), Num<1>>,
    Label<Id("a")>,
    Dec<Mem<Num<0>>>,
    Js<Id("end")>,
    Label<Id("A")>,
    Jz<Id("A")>,
    Label<Id("loop")>,
    Jmp<Id("loop")>,
    Label<Id("end")>
>;
constexpr auto test_id1_res = test_machine::boot<test_id1>();

using test_overflow = Program<
    D<Id("sth"), Num<2147483560>>,
    Label<Id("begin")>,
    Inc<Mem<Num<0>>>,
    Js<Id("end")>,
    Jmp<Id("begin")>,
    Label<Id("end")>>;
//constexpr auto test_overflow_res = test_machine::boot<test_overflow>();

using test_underflow = Program<
    D<Id("mem0"), Num<INT32_MIN>>,
    Add<Mem<Lea<Id("MEM0")>>, Num<-1>>>;
constexpr std::array<int, 4> test_underflow_res = {INT32_MAX, 0, 0, 0};
//static_assert(compare(test_machine::boot<test_underflow>(), test_underflow_res));

using test_finite_loop1 = Program<
    Label<Id("loop")>,
    Dec<Mem<Lea<Id("MEM0")>>>,
    Js<Id("LOOP")>,
    D<Id("mem0"), Num<INT32_MIN + 100>>
>;
//constexpr auto test_finite_loop1_res = test_machine::boot<test_finite_loop1>();

using test_finite_loop2 = Program<
    Label<Id("loop")>,
    Dec<Mem<Lea<Id("MEM0")>>>,
    Js<Id("LOOP")>,
    D<Id("mem0"), Num<10000>>
>;
constexpr auto test_finite_loop2_res = test_machine::boot<test_finite_loop2>();


// tests that should not compile (i.e. template parsing error):

//using test_bad_id1 = Program<
//        Label<Id("%")>>;

//using test_bad_id2 = Program<
//        Label<Id("3324234")>>;

//using test_bad_syntax1 = Program<
//        Inc<Lea<Lea<Id("a")>>>>;


/* tests that should not run as constexpr,
 * i.e. constexpr res = machine::boot<test>() should result in compile-time error.
 * (if they don't even compile then it's good either way) */

using test_infinite_loop = Program<
    Label<Id("loop")>,
    Jmp<Id("LOOP")>>;
//constexpr auto test_infinite_loop_res = test_machine::boot<test_infinite_loop>();

using test_bad_program = Num<4>;
//constexpr auto bad_program = test_machine::boot<test_bad_program>();

using test_size1 = Program<
    Mov<Mem<Num<2137>>, Num<1>>>;
//constexpr auto test_size1_res = test_machine::boot<test_size1>();

using test_syntax1 = Program<
    Num<0>>;
//constexpr auto test_syntax1_res = test_machine::boot<test_syntax1>();

using test_syntax2 = Program<
    Lea<Id("a")>>;
//constexpr auto test_syntax2_res = test_machine::boot<test_syntax2>();

using test_syntax3 = Program<
    Mem<Num<0>>>;
//constexpr auto test_syntax3_res = test_machine::boot<test_syntax3>();

using test_syntax4 = Program<
    Inc<Num<0>>>;
//constexpr auto test_syntax4_res = test_machine::boot<test_syntax4>();

using test_syntax5 = Program<
    Inc<Inc<Num<0>>>>;
//constexpr auto test_syntax5_res = test_machine::boot<test_syntax5>();

// I can't make this test fail, yet I believe it should.
using test_syntax6 = Program<
    Label<4ULL>>;
//constexpr auto test_syntax6_res = test_machine::boot<test_syntax6>();

using test_D_syntax1 = Program<
    D<Id("a"), Mem<Num<1>>>>;
//constexpr auto test_D_syntax1_res = test_machine::boot<test_D_syntax1>();

using test_D_syntax2 = Program<
    D<Id("1"), Num<1>>,
    D<Id("a"), Lea<Id("1")>>>;
//constexpr auto test_D_syntax2_res = test_machine::boot<test_D_syntax1>();

// Basic tests for And, Not, Or, Cmp. ~ab

// AND

using test_results_logical_and = Program<
    D<Id("first"), Num<5>>,
    D<Id("second"), Num<15>>,
    D<Id("third"), Num<13>>,
    D<Id("fourth"), Num<6>>,
    And<Mem<Lea<Id("first")>>, Num<3>>,
    And<Mem<Lea<Id("second")>>, Lea<Id("fourth")>>,
    And<Mem<Lea<Id("third")>>, Mem<Lea<Id("fourth")>>>>;
constexpr std::array<int, 4> test_log_and_res = {1, 3, 4, 6};
static_assert(compare(test_machine::boot<test_results_logical_and>(),
                      test_log_and_res));

// Assumes working conditional jump.
using test_flag_logical_and = Program<
    D<Id("first"), Num<10>>,
    D<Id("second"), Num<2>>,
    And<Mem<Lea<Id("first")>>, Num<8>>,
    Jz<Id("jp3")>,
    And<Mem<Lea<Id("second")>>, Num<8>>,
    Jz<Id("jp3")>,
    Inc<Mem<Num<2>>>,
    Label<Id("jp3")>>;
constexpr std::array<int, 4> test_log_and_flag_res = {8, 0, 0, 0};
static_assert(compare(test_machine::boot<test_flag_logical_and>(),
                      test_log_and_flag_res));

// OR

using test_results_logical_or = Program<
    D<Id("first"), Num<5>>,
    D<Id("second"), Num<10>>,
    D<Id("third"), Num<13>>,
    D<Id("fourth"), Num<6>>,
    Or<Mem<Lea<Id("first")>>, Num<3>>,
    Or<Mem<Lea<Id("second")>>, Lea<Id("fourth")>>,
    Or<Mem<Lea<Id("third")>>, Mem<Lea<Id("fourth")>>>>;
constexpr std::array<int, 4> test_log_or_res = {7, 11, 15, 6};
static_assert(compare(test_machine::boot<test_results_logical_or>(), test_log_or_res));

// Assumes working conditional jump.
using test_flag_logical_or = Program<
    D<Id("first"), Num<10>>,
    D<Id("second"), Num<0>>,
    Or<Mem<Lea<Id("first")>>, Num<8>>,
    Jz<Id("jp3")>,
    Or<Mem<Lea<Id("second")>>, Num<0>>,
    Jz<Id("jp3")>,
    Inc<Mem<Num<2>>>,
    Label<Id("jp3")>>;
constexpr std::array<int, 4> test_log_or_flag_res = {10, 0, 0, 0};
static_assert(compare(test_machine::boot<test_flag_logical_or>(),
                      test_log_or_flag_res));

// NOT

using test_results_logical_not = Program<
    D<Id("first"), Num<10>>,
    D<Id("second"), Num<-10>>,
    D<Id("third"), Num<0>>,
    Not<Mem<Lea<Id("first")>>>,
    Not<Mem<Lea<Id("second")>>>,
    Not<Mem<Lea<Id("third")>>>>;
constexpr std::array<int, 4> test_log_not_res = {-11, 9, -1, 0};
static_assert(compare(test_machine::boot<test_results_logical_not>(),
                      test_log_not_res));

// Assumes working conditional jump.
using test_flag_logical_not = Program<
    D<Id("first"), Num<10>>,
    D<Id("second"), Num<-1>>,
    Not<Mem<Lea<Id("first")>>>,
    Jz<Id("jp3")>,
    Not<Mem<Lea<Id("second")>>>,
    Jz<Id("jp3")>,
    Inc<Mem<Num<2>>>,
    Label<Id("jp3")>>;
constexpr std::array<int, 4> test_log_not_flag_res = {-11, 0, 0, 0};
static_assert(compare(test_machine::boot<test_flag_logical_not>(),
                      test_log_not_flag_res));

// CMP
// Only one test, since CMP only sets flags.
//using test_bad_id1 = Program<
//        Label<Id("%")>>;

//using test_bad_id2 = Program<
//        Label<Id("3324234")>>;

//using test_bad_syntax1 = Program<
//        Inc<Lea<Lea<Id("a")>>>>;

using test_compare = Program<
    D<Id("first"), Num<10>>,
    D<Id("second"), Num<1>>,
    D<Id("third"), Num<0>>,
    Cmp<Mem<Lea<Id("first")>>, Mem<Lea<Id("second")>>>,
    Cmp<Mem<Lea<Id("first")>>, Lea<Id("second")>>,
    Cmp<Mem<Lea<Id("first")>>, Num<5>>,
    Cmp<Lea<Id("third")>, Mem<Lea<Id("second")>>>,
    Cmp<Lea<Id("third")>, Lea<Id("second")>>,
    Cmp<Lea<Id("third")>, Num<0>>,
    Cmp<Num<42>, Mem<Lea<Id("second")>>>,
    Cmp<Num<42>, Lea<Id("second")>>,
    Cmp<Num<42>, Num<5>>,
    // Should not jump
    Jz<Id("42")>,
    Js<Id("42")>,
    Inc<Mem<Lea<Id("third")>>>,
    Cmp<Num<10>, Num<10>>,
    // Should jump only on Jz, to label "21"
    Js<Id("42")>,
    Jz<Id("21")>,
    Inc<Mem<Lea<Id("third")>>>,
    Label<Id("21")>,
    Cmp<Num<5>, Num<10>>,
    // Should jump only on Js, to label "37"
    Jz<Id("42")>,
    Js<Id("37")>,
    Inc<Mem<Lea<Id("third")>>>,
    Label<Id("37")>,
    Inc<Mem<Num<3>>>,
    Label<Id("42")>>;
constexpr std::array<int, 4> test_compare_res = {10, 1, 1, 1};
static_assert(compare(test_machine::boot<test_compare>(), test_compare_res));

// Other tests
using tests_correct_ids = Program<
    Label<Id("a")>,
    Label<Id("AA")>,
    Label<Id("aAzZkE")>,
    Label<Id("0")>,
    Label<Id("213742")>,
    Label<Id("42")>,
    Label<Id("a1H55m")>>;

// These should not work:

//using tests_incorrect_id_1 = Program<Label<Id("aaaaaaa")>>;
//using tests_incorrect_id_2 = Program<Label<Id("1111111")>>;
//using tests_incorrect_id_3 = Program<Label<Id("pioter zmienilem sie w id, tekst dolny")>>;
//using tests_incorrect_id_4 = Program<Label<Id("")>>;
//using tests_incorrect_id_5 = Program<Label<Id("-*/=-?")>>;
//using tests_incorrect_id_6 = Program<Label<Id(";<>.,")>>;
//using tests_incorrect_id_7 = Program<Label<Id(" ")>>;
//using tests_incorrect_id_8 = Program<Label<Id("pa paj")>>;

//using tests_incorrect_num = Program<D<Id("n"), Num<"21">>>;

// Should work, but it's a side effect
using tests_corr_lea = Program<Lea<1488>>;



int main() {
//
//
//test_machine::boot<test_finite_loop>();
//test_machine::boot<test_id1>();
//
//auto arr = test_machine::boot<test_id1>();
//std::cout << INT32_MAX << std::endl;
for (auto itr : test_declarations_res)
    std::cout << (int) itr << std::endl;
}