
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <raspberry/raspberry.hpp>

#include <string>

RASPBERRY_DECL_METHOD(FuncConcept, func);
RASPBERRY_DECL_METHOD(SquareConcept, square);

using AnyFunc = raspberry::Any<FuncConcept<int()const>,SquareConcept<float(float)>>;

struct SomeFunc {
    int func() const {
        return 42;
    }

    float square(float x) {
        return x*x;
    }
};

TEST_CASE("Objects can be stored in Any", "[raspberry]") {
    AnyFunc f = SomeFunc{};

    REQUIRE(f.func() == 42);
    REQUIRE(f.square(12) == 144);
}

struct negative_test_assign {
    template <typename AF>
    static decltype( AF(std::declval<const AF&>()), bool{} ) test(int) { return false; }

    template <typename AF>
    static bool test(long) { return true; }
};

TEST_CASE("Any cannot be stored in Any or copied", "[raspberry]") {
    REQUIRE(negative_test_assign::test<AnyFunc>(int{}));
}

RASPBERRY_DECL_METHOD(RefDetectConcept, ref_detect);

using AnyRefDetector = raspberry::Any<RefDetectConcept<void(int)>>;

struct RefDetector {
    int value = 0;

    void ref_detect(int x) {
        value = x;
    }
};

TEST_CASE("Objects are copied by default", "[raspberry]") {
    RefDetector rd;
    REQUIRE(rd.value == 0);

    AnyRefDetector ard = rd;
    REQUIRE(rd.value == 0);

    ard.ref_detect(42);
    REQUIRE(rd.value == 0);
}

TEST_CASE("std::reference_wrapper is used to capture by reference", "[raspberry]") {
    RefDetector rd;
    REQUIRE(rd.value == 0);

    AnyRefDetector ard = std::ref(rd);
    REQUIRE(rd.value == 0);

    ard.ref_detect(42);
    REQUIRE(rd.value == 42);
}

RASPBERRY_DECL_METHOD(SetStringConcept, set_string);

using AnySetString = raspberry::Any<
        SetStringConcept< void(const std::string&) >,
        SetStringConcept< void(const char*) >
>;

struct StringSetter {
    std::string value;

    void set_string(const std::string& s) {
        value = s;
    }

    void set_string(const char* s) {
        value = s;
    }
};

TEST_CASE("Methods can be overloaded", "[raspberry]") {
    StringSetter s;
    AnySetString a = std::ref(s);

    a.set_string("char[]");
    REQUIRE(s.value == "char[]");

    a.set_string(std::string("std::string"));
    REQUIRE(s.value == "std::string");

}

RASPBERRY_DECL_METHOD(MaybeConstGetter, get);

using AnyMaybeConstGetter = raspberry::Any<
        MaybeConstGetter< int&() >,
        MaybeConstGetter< const int&() const >
>;

struct SomeMaybeConstGetter {
    int value;
    int& get() { return value; }
    const int& get() const { return value; }
};

TEST_CASE("Const and non-const overloads can coexist", "[raspberry]") {
    SomeMaybeConstGetter s;
    AnyMaybeConstGetter a = std::ref(s);

    s.value = 7;
    REQUIRE(s.value == 7);

    a.get() = 42;
    REQUIRE(s.value == 42);

    const auto& ac = a;
    REQUIRE(ac.get() == 42);
    REQUIRE(std::is_const<std::remove_reference_t<decltype(ac.get())>>::value);
}

using AnyMaybeConstGetterReversed = raspberry::Any<
        MaybeConstGetter< const int&() const >,
        MaybeConstGetter< int&() >
>;

TEST_CASE("Const and non-const overloads can come in any order", "[raspberry]") {
    SomeMaybeConstGetter s;
    AnyMaybeConstGetterReversed a = std::ref(s);

    s.value = 7;
    REQUIRE(s.value == 7);

    a.get() = 42;
    REQUIRE(s.value == 42);

    const auto& ac = a;
    REQUIRE(ac.get() == 42);
    REQUIRE(std::is_const<std::remove_reference_t<decltype(ac.get())>>::value);
}

RASPBERRY_DECL_METHOD(ConstTester, c_func);

using AnyConstTester = raspberry::Any< ConstTester< void() > >;

struct SomeConstTester {
    void c_func() const {}
};

TEST_CASE("Const methods can be called from non-const concepts", "[raspberry]") {
    AnyConstTester ac = SomeConstTester{};
    ac.c_func();
    REQUIRE(true);
}
