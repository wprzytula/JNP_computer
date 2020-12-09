#ifndef INC_4_COMPUTER_H
#define INC_4_COMPUTER_H

#include <cstddef>
#include <array>
//#include <iostream>
#include <cassert>
#include <type_traits>

using num_id_t = unsigned long long;

template <size_t n, typename T>
using memory_t = std::array<T, n>;

template <size_t n>
using vars_t = std::array<num_id_t, n>;

// mapa identyfikatorów na miejsca w pamięci

// https://stackoverflow.com/questions/16490835/how-to-build-a-compile-time-key-value-store

template <num_id_t kk, size_t vv>
struct kv {
    static constexpr num_id_t k = kk;
    static constexpr size_t v = vv;
};

template <typename...>
struct ct_map;

template <>
struct ct_map<> {
    template<num_id_t>
    struct get {
        static constexpr auto val = 0; // default case
    };

    template <num_id_t kk, size_t vv>
    struct add {
        using result = ct_map<kv<kk, vv>>;
    };
};

template<num_id_t k, size_t v, typename... rest>
struct ct_map<kv<k, v>, rest...> {
    template<num_id_t kk>
    struct get {
        static constexpr auto val =
                (kk == k) ?
                v :
                ct_map<rest...>::template get<kk>::val;
    };
    template <num_id_t kk, size_t vv>

    struct add {
        using result = ct_map<kv<kk, vv>, kv<k, v>, rest...>;
    };
};




// TMPAsm Elements

constexpr num_id_t Id(const char* id) {
    num_id_t num_id = 0ULL;
    unsigned i = 0;
    assert(id[0] != '\0' && ((id[0] >= '0' && id[0] <= '9') ||
           (id[0] >= 'A' && id[0] <= 'Z') || (id[0] >= 'a' && id[0] <= 'z')));
    while (id[i] != '\0') {
        num_id += ((num_id_t)(unsigned char)(id[i] >= 'a' ? id[i] - ('a' - 'A') : id[i]))
                << (8U * i);
        ++i;
        assert(i < 7);
    }
    return num_id;
}

template <auto num>
struct Num {
    template <size_t n, typename T>
    static constexpr auto rval(const vars_t<n>&, const memory_t <n, T>&) {
//        static_assert(std::is_integral<decltype(num)>::value); -> to zbędne, bo n jako parametr
//                                                                    array załatwia sprawę
        return num;
    }
};

template <typename Addr>
struct Mem {
    template <size_t n, typename T>
    static constexpr auto* addr(vars_t<n> vars, std::array<T, n>& memory) {
//        static_assert(Addr::template rval<n, T>(memory) < n);
        return &memory[Addr::template rval<n, T>(vars, memory)];
    }
    template <size_t n, typename T>
    static constexpr auto const& rval(vars_t<n> vars, const memory_t<n, T>& memory) {
        return memory[Addr::template rval<n, T>(vars, memory)];
    }
};

template <num_id_t num_id>
struct Lea {
    template<size_t n, typename T>
    static constexpr size_t rval(const vars_t<n>& vars, const memory_t<n, T>&) {
        for (size_t i = 0; i < n; ++i) {
            if (vars[i] == num_id)
                return i;
        }
        return 0;
//        assert(false);
    }
};

// TMPAsm Instructions

template <num_id_t id, typename Value>
struct D {
//    static constexpr bool declaration = true;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars,
                                  memory_t<n, T>& memory) {
        size_t i = 0;
        while (i < n) {
            if (vars[i] == 0) {
                vars[i] = id;
                memory[i] = Value::rval(vars, memory);
                return;
            }
            ++i;
        }
        assert(false);
    }
};

template <typename LValue, typename RValue>
struct Mov {
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t<n, T>& memory, bool&, bool&) {
        *LValue::template addr<n, T>(vars, memory) =
                RValue::template rval<n, T>(vars, memory);
    }
};

template <typename LValue, typename RValue>
struct Add {
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory, bool& ZF, bool& SF) {
        auto result = *LValue::template addr<n, T>(vars, memory) +=
                RValue::template rval<n, T>(vars, memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <typename LValue, typename RValue>
struct Sub {
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n> vars, memory_t<n, T>& memory, bool& ZF, bool& SF) {
        auto result = *LValue::template addr<n, T>(vars, memory) -=
                RValue::template rval<n, T>(vars, memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <typename LValue>
struct Inc {
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory,
                                  bool& ZF, bool& SF) {
        auto result = ++*LValue::template addr<n, T>(vars, memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <typename LValue>
struct Dec {
    template <size_t n, typename T>
    static constexpr void execute(std::array<T, n>& memory, bool& ZF, bool& SF) {
        auto result = --*LValue::template addr<n, T>(memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <size_t n, typename T, typename T2>
struct isDeclaration {
    static constexpr bool result = false;
};

template<size_t n, typename T>
using execute_t = void (vars_t<n>&, memory_t <n, T>&, bool&, bool&);

template<size_t n, typename T>
using declare_t = void (vars_t<n>&, memory_t <n, T>&);

template <size_t n, typename T>
struct isDeclaration <n, T, execute_t<n, T>> {
static constexpr bool result = false;
};

template <size_t n, typename T>
struct isDeclaration <n, T, declare_t<n, T>> {
    static constexpr bool result = true;
};


/*
template <typename Instr>
struct isDeclaration2 {
    using a = decltype(Instr::declaration);
    static constexpr bool result = true;
};
*/


template <typename...>
struct Program;

template <>
struct Program <> {
    template <size_t n, typename T>
    static constexpr void declare(vars_t<n>&, memory_t<n, T>&) {}
    template <size_t n, typename T, typename>
    static constexpr void run(vars_t<n>&,
                              memory_t<n, T>&, bool&, bool&) {}
};

template <typename Instr, typename ...rest>
struct Program <Instr, rest...> {
    template <size_t n, typename T>
    static constexpr void declare(vars_t<n>& vars, memory_t<n, T>& memory) {
        constexpr bool declaration = isDeclaration<
                n, T, decltype(Instr::template execute<n, T>)>::result;
        if constexpr (declaration)
            Instr::template execute<n, T>(vars, memory);
        Program<rest...>::template declare<n, T>(vars, memory);
    }

    template <size_t n, typename T, typename P>
    static constexpr void run(vars_t<n>& vars,
            memory_t<n, T>& memory, bool& ZF, bool& SF) {
        constexpr bool declaration = isDeclaration<
                n, T, decltype(Instr::template execute<n, T>)>::result;
        if constexpr (!declaration)
            Instr::template execute<n, T>(vars, memory, ZF, SF);
        Program<rest...>::template run<n, T>(vars, memory, ZF, SF);
    }
};


template <size_t n, typename T>
struct Computer {
    template <typename P>
    static constexpr memory_t<n, T> boot() {
        memory_t<n, T> memory {0};
        vars_t<n> vars {0};
        bool ZF = false;
        bool SF = false;
        P::template declare<n, T>(vars, memory);
        P::template run<n, T, P>(vars, memory, ZF, SF);
        return memory;
    }
};







// wzorce templatkowe do nauki

template <typename...>
struct ct_map2;

template <>
struct ct_map2<> {};

template<num_id_t k, size_t v, typename... rest>
struct ct_map2<kv<k, v>, rest...> {};


#endif //INC_4_COMPUTER_H
