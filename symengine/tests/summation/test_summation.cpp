#include "catch.hpp"

#include <symengine/summation.h>
#include <symengine/visitor.h>

using SymEngine::dummy;
using SymEngine::symbol;
using SymEngine::zero;
using SymEngine::one;
using SymEngine::eq;
using SymEngine::integer;
using SymEngine::div;
using SymEngine::add;

TEST_CASE("Test simple", "[summation]")
{
    auto i = symbol("i");
    auto n = symbol("n");

    // stupid cases
    // Sum(i = [0..n]){ 0 } = 0
    REQUIRE(eq(*summation(zero, i, zero, n), *zero));
    // Sum(i = [0..5]){ 1 } = 6
    REQUIRE(eq(*summation(one, i, zero, integer(5)), *integer(6)));

    // simples
    // Sum(i = [0..n]){ 1 } = n + 1
    REQUIRE(eq(*summation(one, i, zero, n), *add(n, one)));
    // Sum(i = [0..n]){ 5 } = 5n + 5
    REQUIRE(eq(*summation(integer(5), i, zero, n), *mul(integer(5), add(n, one))));

    // bernoulli cases
    // Sum(i = [0..n]) { i ** 2 } = n ** 3 / 3 + n ** 2 / 2 + n / 6
    REQUIRE(eq(
        *summation(pow(i, integer(2)), i, zero, n),
        *add({ div(pow(n, integer(3)), integer(3)), div(pow(n, integer(2)), integer(2)), div(n, integer(6)) })
        ));


}
