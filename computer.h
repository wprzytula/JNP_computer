/*
 * Pytania do Peczara:
 * 1) czy opakować funkcje i typy pomocnicze w jakiś namespace?
 *
 *
 *
 *
 * */



#ifndef INC_4_COMPUTER_H
#define INC_4_COMPUTER_H

#include <cstddef>
#include <array>
#include <cassert>
#include <type_traits>
#include <limits>

// TODO: wywalić to
#pragma clang diagnostic push
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"


using num_id_t = unsigned long long;

template <size_t n, typename T>
using memory_t = std::array<T, n>;

template <size_t n>
using vars_t = std::array<num_id_t, n>;


// TMPAsm Elements

// [TODO]: Consider changing to enum class.
enum instruction_t {JUMP, DECLARATION, LABEL, INSTRUCTION};
enum element_t {NUM, LEA, MEM};

// todo: może wymagać komentarzy
constexpr num_id_t Id(const char* id) {
    num_id_t num_id = 0ULL;
    unsigned i = 0;
    assert(id[0] != '\0' && ((id[0] >= '0' && id[0] <= '9') ||
           (id[0] >= 'A' && id[0] <= 'Z') || (id[0] >= 'a' && id[0] <= 'z')));
    while (id[i] != '\0') {
        num_id += ((num_id_t) (unsigned char) (id[i] >= 'a' ? id[i] - ('a' - 'A') : id[i]))
                << (8U * i);
        ++i;
        assert(i < 7);
    }
    return num_id;
}

template <auto num>
struct Num {
    static constexpr element_t el_type = NUM;
    template <size_t n, typename T>
    static constexpr auto rval(const vars_t<n>&, const memory_t <n, T>&) {
        return T(num);
    }
};

template <typename Addr>
struct Mem {
    static constexpr element_t el_type = MEM;
    template <size_t n, typename T>
    static constexpr auto* addr(vars_t<n> vars, memory_t <n, T>& memory) {
        return &memory[Addr::template rval<n, T>(vars, memory)];
    }
    template <size_t n, typename T>
    static constexpr auto const& rval(vars_t<n> vars, const memory_t<n, T>& memory) {
        return memory[Addr::template rval<n, T>(vars, memory)];
    }
};

template <num_id_t num_id>
struct Lea {
    static constexpr element_t el_type = LEA;
    template<size_t n, typename T>
    static constexpr size_t rval(const vars_t<n>& vars, const memory_t<n, T>&) {
        for (size_t i = 0; i < n; ++i) {
            if (vars[i] == num_id)
                return i;
        }
        assert(false);
    }
};


// TMPAsm Instructions

struct Instruction {};

template <num_id_t id, typename Value>
struct D : Instruction {
    static_assert(Value::el_type == NUM);
    static constexpr instruction_t ins_type = DECLARATION;
    template <size_t n, typename T>
    static constexpr void declare(vars_t<n>& vars, memory_t<n, T>& memory) {
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
struct Mov : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t<n, T>& memory, bool&, bool&) {
        *LValue::template addr<n, T>(vars, memory) =
                RValue::template rval<n, T>(vars, memory);
    }
};

template <typename T>
constexpr T add(T op1, T op2) {
    if (op1 > 0 && op2 > 0) { // check for overflow - possible only when both operands are positive
        T to_max = std::numeric_limits<T>::max() - op2;
        if (to_max < op1)
            return std::numeric_limits<T>::min() + op2 - (std::numeric_limits<T>::max() - op1) - 1;
        else
            return op1 + op2;
    } else if (op1 < 0 && op2 < 0) { // check for underflow
        T to_min = op2 - std::numeric_limits<T>::min();
        if (op2 < to_min) {
            T temp = op1 - std::numeric_limits<T>::min();
            return std::numeric_limits<T>::max() + op2 + temp; }
        else
            return op1 + op2;
    } else {
        return op1 + op2;
    }
}

template <typename T>
constexpr T subtract(T op1, T op2) {
    if (op2 == std::numeric_limits<T>::min()) {
        // TODO
    } else {
        return add<T>(op1, -op2);
    }
}

template <typename LValue, typename RValue>
struct Add : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory, bool& ZF, bool& SF) {
        T* lval = LValue::template addr<n, T>(vars, memory);
        T rval = RValue::template rval<n, T>(vars, memory);

        T result = *lval = add<T>(*lval, rval);
//        T result = *lval += rval;
        ZF = result == (T)0;
        SF = result < (T)0;
    }
};

template <typename LValue, typename RValue>
struct Sub : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n> vars, memory_t<n, T>& memory, bool& ZF, bool& SF) {
        T* lval = LValue::template addr<n, T>(vars, memory);
        T rval = RValue::template rval<n, T>(vars, memory);
        T result = *lval -= rval;
        ZF = result == (T)0;
        SF = result < (T)0;
    }
};

template <typename LValue>
using Inc = Add<LValue, Num<1>>;

template <typename LValue>
using Dec = Sub<LValue, Num<1>>;

// TMPAsm logical operations.

template <typename LValue, typename RValue>
struct And : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory, bool& ZF, bool&) {
        auto result = (*LValue::template addr<n, T>(vars, memory) &=
                RValue::template rval<n, T>(vars, memory));
        ZF = (result == 0);
    }
};

template <typename LValue, typename RValue>
struct Or : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory, bool& ZF, bool&) {
        auto result = (*LValue::template addr<n, T>(vars, memory) |=
                RValue::template rval<n, T>(vars, memory));
        ZF = (result == 0);
    }
};

template <typename LValue>
struct Not : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n>& vars, memory_t <n, T>& memory, bool& ZF, bool&) {
        auto result = (*LValue::template addr<n, T>(vars, memory) =
                ~(*LValue::template addr<n, T>(vars, memory)));
        ZF = (result == 0);
    }
};

// TMPAsm compare operation.

template <typename RValue1, typename RValue2>
struct Cmp : Instruction {
    static constexpr instruction_t ins_type = INSTRUCTION;
    template <size_t n, typename T>
    static constexpr void execute(vars_t<n> vars, memory_t<n, T>& memory, bool& ZF, bool& SF) {
        auto result = RValue1::template rval<n, T>(vars, memory) -
                RValue2::template rval<n, T>(vars, memory);
        ZF = (result == 0);
        SF = (result < 0);
    }
};


template <num_id_t id>
struct Label : Instruction {
    static constexpr instruction_t ins_type = LABEL;
    static constexpr num_id_t label = id;
};

template <num_id_t L>
struct Jmp : Instruction {
    static constexpr instruction_t ins_type = JUMP;
    static constexpr num_id_t label = L;
    template <size_t n, typename T>
    static constexpr bool should_jump(const bool&, const bool&) {
        return true;
    }
};

template <num_id_t L>
struct Js : Instruction {
    static constexpr instruction_t ins_type = JUMP;
    static constexpr num_id_t label = L;
    template <size_t n, typename T>
    static constexpr bool should_jump(const bool&, const bool& SF) {
        return SF;
    }
};

template <num_id_t L>
struct Jz : Instruction {
    static constexpr instruction_t ins_type = JUMP;
    static constexpr num_id_t label = L;
    template <size_t n, typename T>
    static constexpr bool should_jump(const bool& ZF, const bool&) {
        return ZF;
    }
};

template <typename...>
struct Program;

template <>
struct Program <> {
    template <size_t n, typename T>
    static constexpr void declare(vars_t<n>&, memory_t<n, T>&) {}
    template <size_t n, typename T, typename P>
    static constexpr void run(vars_t<n>&, memory_t<n, T>&,
                              bool&, bool&) {}
    template <size_t n, typename T, typename P>
    static constexpr void jump(vars_t<n>&, memory_t<n, T>&,
                               bool&, bool&) {
        assert(false);
    }
};

template <typename Line, typename ...rest>
struct Program <Line, rest...> {
    static_assert(std::is_base_of<Instruction, Line>::value);

    template <size_t n, typename T>
    static constexpr void declare(vars_t<n>& vars, memory_t<n, T>& memory) {
        if constexpr (Line::ins_type == DECLARATION)
            Line::template declare<n, T>(vars, memory);
        Program<rest...>::template declare<n, T>(vars, memory);
    }

    template <size_t n, typename T, typename P>
    static constexpr void run(vars_t<n>& vars,
            memory_t<n, T>& memory, bool& ZF, bool& SF) {
        if constexpr (Line::ins_type == JUMP) {
            if (Line::template should_jump<n, T>(ZF, SF))
                P::template jump<n, T, P, Line::label>(vars, memory, ZF, SF);
            else
                Program<rest...>::template run<n, T, P>(vars, memory, ZF, SF);
        }
        else {
            if constexpr (Line::ins_type == INSTRUCTION)
                Line::template execute<n, T>(vars, memory, ZF, SF);
            Program<rest...>::template run<n, T, P>(vars, memory, ZF, SF);
        }
    }

    template <size_t n, typename T, typename P, num_id_t label>
    static constexpr void jump(vars_t<n>& vars, memory_t<n, T>& memory,
                               bool& ZF, bool& SF) {
        if constexpr (Line::ins_type == LABEL) {
            if constexpr (Line::label == label)
                run<n, T, P>(vars, memory, ZF, SF);
            else
                Program<rest...>::template jump<n, T, P, label>(vars, memory, ZF, SF);
        } else
            Program<rest...>::template jump<n, T, P, label>(vars, memory, ZF, SF);
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

#endif //INC_4_COMPUTER_H

#pragma clang diagnostic pop