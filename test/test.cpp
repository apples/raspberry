
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <raspberry/raspberry.hpp>

DECL_ERASURE_MEMBER_CONCEPT(FuncConcept, func);
DECL_ERASURE_MEMBER_CONCEPT(SquareConcept, square);

using AnyFunc = Raspberry::Any<FuncConcept<int()>,SquareConcept<float(float)>>;

struct SomeFunc {
    int func() {
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
