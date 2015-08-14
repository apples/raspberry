
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <raspberry/raspberry.hpp>

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
