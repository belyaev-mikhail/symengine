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
using SymEngine::Expression;
using SymEngine::Symbol;

SymEngine::RCP<const SymEngine::Integer> operator"" _s (unsigned long long i) {
    return integer(i);
}

SymEngine::RCP<const SymEngine::Symbol> operator"" _s (const char* v, unsigned long sz) {
    return symbol(std::string(v, sz));
}

inline Expression summation(const Expression &f, const Expression &i, const Expression &from, const Expression &to) {
    SYMENGINE_ASSERT(SymEngine::is_a<SymEngine::Dummy>(*i.get_basic())
        or SymEngine::is_a<SymEngine::Symbol>(*i.get_basic()))

    return Expression(
        summation(f.get_basic(), SymEngine::rcp_static_cast<const Symbol>(i.get_basic()), from.get_basic(), to.get_basic())
        );
}

inline Expression pow(const Expression &a, const Expression &b) {
    return Expression(pow(a.get_basic(), b.get_basic()));
}


TEST_CASE("Test simple", "[concrete]")
{
    using SymEngine::Expression;
    auto i = Expression(dummy());
    auto n = Expression(dummy());



    // stupid cases
    // Sum(i = [0..n]){ 0 } = 0
    REQUIRE(summation(0, i, 0, n) == 0);
    // Sum(i = [0..5]){ 1 } = 6
    REQUIRE(summation(1, i, 0, 5) == 6);

    // simples
    // Sum(i = [0..n]){ 1 } = n + 1
    REQUIRE(summation(1, i, 0, n) == n + 1);
    // Sum(i = [0..n]){ 5 } = 5n + 5
    REQUIRE(summation(5, i, 0, n) == 5 * (n + 1));

    // bernoulli cases
    // Sum(i = [0..n]) { i ** 2 } = n ** 3 / 3 + n ** 2 / 2 + n / 6
    REQUIRE(summation(pow(i, 2), i, 0, n) == pow(n, 3) / 3 + pow(n, 2) / 2 + n / 6);

    // Sum(i = [0..n]){ i } = (n ** 2 + n) / 2
    REQUIRE(summation(i, i, 0, n) == n * n / 2 + n / 2);
}

TEST_CASE("Test complex", "[concrete]")
{
    using namespace SymEngine;

    auto i = Expression(dummy());
    auto j = Expression(dummy());
    auto n = Expression(dummy());

    // Sum(i = [0..n]){ 1 } = n + 1
    REQUIRE(summation(1, i, 0, n) == n + 1);

    REQUIRE(summation(summation(1, i, 0, j), j, 0, n) == pow(n, 2) / 2 + n * 3 / 2 + 1);

}
