#ifndef INC_4_COMPUTER_H
#define INC_4_COMPUTER_H

//#include <cstddef>
#include <array>
#include <iostream>
#include <cassert>
#include <type_traits>

using num_id_t = unsigned long long;

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

constexpr num_id_t Id(const char id[7]) {
    num_id_t num_id = 0ULL;
    unsigned i = 0;
    assert(id[0] != '\0');
    while (id[i] != '\0') {
        num_id += ((num_id_t)(unsigned char) std::toupper(id[i])) << (8U * i);
        ++i;
        assert(i < 7);
    }
    return num_id;
}

template <auto num>
struct Num {
    template <size_t n, typename T>
    static constexpr auto rval(const std::array<T, n>&) {
        static_assert(std::is_integral<decltype(num)>::value);
        return num;
    }
};

template <typename Addr>
struct Mem {
    template <size_t n, typename T>
    static constexpr auto* addr(std::array<T, n>& memory) {
//        static_assert(Addr::template rval<n, T>(memory) < n);
        return &memory[Addr::template rval<n, T>(memory)];
    }
    template <size_t n, typename T>
    static constexpr auto& rval(const std::array<T, n>& memory) {
        return memory[Addr::template rval<n, T>(memory)];
    }
};

template <num_id_t id>
struct Lea {
    template<typename id_map>
    static constexpr size_t rval() {
        return id_map::template get<id>::val;
    }
};

// TMPAsm Instructions

template <typename LValue, typename RValue>
struct Mov {
    template <size_t n, typename T>
    static constexpr void execute(std::array<T, n>& memory, bool&, bool&) {
        *LValue::template addr<n, T>(memory) =
                RValue::template rval<n, T>(memory);
    }
};

template <typename LValue, typename RValue>
struct Add {
    template <size_t n, typename T>
    static constexpr void execute(std::array<T, n>& memory, bool& ZF, bool& SF) {
        auto result = *LValue::template addr<n, T>(memory) +=
                RValue::template rval<n, T>(memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <typename LValue, typename RValue>
struct Sub {
    template <size_t n, typename T>
    static constexpr void execute(std::array<T, n>& memory, bool& ZF, bool& SF) {
        auto result = *LValue::template addr<n, T>(memory) -=
                RValue::template rval<n, T>(memory);
        ZF = result == 0;
        SF = result < 0;
    }
};

template <typename LValue>
struct Inc {
    template <size_t n, typename T>
    static constexpr void execute(std::array<T, n>& memory, bool& ZF, bool& SF) {
        auto result = ++*LValue::template addr<n, T>(memory);
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


template <typename...>
struct Program;

template <>
struct Program <> {
    template <size_t n, typename T>
    static constexpr void run(std::array<T, n>&, bool&, bool&) {}
};

template <typename Instr, typename ...rest>
struct Program <Instr, rest...> {
    template <size_t n, typename T>
    static constexpr void run(std::array<T, n>& memory, bool& ZF, bool& SF) {
        Instr::template execute<n, T>(memory, ZF, SF);
        Program<rest...>::template run<n, T>(memory, ZF, SF);
    }
};

//template <typename Jump, typename ...rest>
//struct Program <Jump, rest...> {
//    template <auto& memory, bool& ZF, bool& SF>
//    constexpr void run() {
//        Jump::template jump<memory, ZF, SF>();
//        Program<rest...>::run();
//    }
//};


template <size_t n, typename T>
struct Computer {
    using memory_t = std::array<T, n>;

    template <typename P>
    static constexpr memory_t boot() {
        memory_t memory = {0};
        bool ZF = false;
        bool SF = false;
        P::template run<n, T>(memory, ZF, SF);
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
