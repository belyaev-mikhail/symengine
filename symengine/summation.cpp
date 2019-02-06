#include <symengine/add.h>
#include <symengine/pow.h>
#include <symengine/complex.h>

#include <symengine/summation.h>
#include <symengine/visitor.h>

namespace SymEngine
{

static RCP<const Basic> one_half() {
    static auto result = rational(1, 2);
    return result;
}

struct integer_class_hash {
    size_t operator() (const integer_class& ic) const noexcept {
        return static_cast<size_t>(mp_get_ui(ic)) * static_cast<size_t>(mp_sign(ic));
    }
};

RCP<const Basic> bernoulli(const RCP<const Number> &n, const RCP<const Basic> &sym = null) {
    static std::unordered_map<unsigned long , RCP<const Number>> cache = {
        {0, one}, {2, rational(1, 6)}, {4, rational(-1, 30)}
    };

    static std::unordered_map<unsigned long, unsigned long> highest = {
        {0, 0}, {2, 2}, {4, 4}
    };

    if(is_a_Number(*n)) {
        if(n->is_zero()) return one;
        if(is_a<Integer>(*n) and n->is_positive()) {
            auto&& nint = down_cast<const Integer&>(*n);
            if(nint.is_one()) {
                return sym.is_null() ? neg(one_half()) : sub(sym, one_half());
            }

            if(sym.is_null()) {
                auto mpn = nint.as_uint();
                if(mpn % 2 == 1) return zero;

                auto case_ = mpn % 6;
                auto highest_cached = highest[case_];
                if(mpn <= highest_cached) return cache[mpn];

                RCP<const Number> b;
                for(auto i = highest_cached + 6; i < mpn + 6; i += 6) {
                    b = bernoulli(i);
                    cache[i] = b;
                    highest[case_] = i;
                }
                return b;
            } else { // !sym.is_null()
                auto mpn = nint.as_uint();
                vec_basic res{};
                for(unsigned long k_ = 0; k_ < mpn + 1; ++k_) {
                    auto k = integer(k_);
                    auto binom = binomial(nint, k_);
                    auto recur = bernoulli(k);
                    res.push_back(mul({ binom, recur, pow(sym, sub(n, k))}));
                }
                return add(res);
            }
        } else return null;
    } else return null;
};

RCP<const Basic> sym_harmonic(const RCP<const Basic>& n, const RCP<const Basic>& m_ = null) {
    struct harmonic_function {
        integer_class m;
        vec_basic cache;

        RCP<const Basic> operator()(unsigned long n) {
            while(cache.size() <= n) {
                auto&& prev = cache.back();
                cache.push_back(add(prev, div(one, pow(integer(n), integer(m)))));
            }
            return cache[n];
        }
    };

    static std::unordered_map<integer_class, harmonic_function, integer_class_hash> functions;

    RCP<const Basic> m = m_ == null ? static_cast<RCP<const Basic>>(one) : m_;
    if(eq(*m, *zero)) return n;
    if(eq(*n, *Inf) and is_a_Number(*m)) {
        auto&& mnumber = down_cast<const Number&>(*m);
        if(mnumber.is_negative()) return Nan;
        if(unified_compare(m, one) < 0) return Inf;
        if(unified_compare(m, one) >= 0) return zeta(m);
    }

    if(eq(*n, *zero)) return zero;

    if(is_a<Integer>(*n)) {
        auto&& n_ = down_cast<const Integer&>(*n);
        if((n_.is_positive() or n_.is_zero()) and is_a<Integer>(*m)) {
            auto&& m_ = down_cast<const Integer&>(*m).as_integer_class();
            auto&& hf = functions[m_] = harmonic_function { m_, { zero } };
            return hf(n_.as_uint());
        }
    }

    return null;
}

RCP<const Basic> summation_direct(const RCP<const Basic>& f,
                           const RCP<const Symbol>& index,
                           const RCP<const Basic>& from,
                           const RCP<const Basic>& to,
                           const Integer& dif) {
    vec_basic res{};
    for(auto j = integer_class(0); j < dif.as_integer_class(); ++j) {
        res.push_back(f->subs({{index, add(from, integer(j))}}));
    }
    return add(res);
}


RCP<const Basic> telescopic(const RCP<const Basic> &l, const RCP<const Basic> &r, const RCP<const Symbol> &index,
                            const RCP<const Basic> &from, const RCP<const Basic> &to) {
    if(is_a<Add>(*l) || is_a<Add>(*r)) return SymEngine::null;



    return SymEngine::null;
}

RCP<const Basic> summation_symbolic(const RCP<const Basic>& f,
                                    const RCP<const Symbol>& index,
                                    const RCP<const Basic>& from,
                                    const RCP<const Basic>& to) {
    if(is_a<Mul>(*f)) {
        auto&& fmul = down_cast<const Mul&>(*f);

        auto&& coef = fmul.get_coef();
        auto&& dict = fmul.get_dict();

        map_basic_basic ldict{};
        map_basic_basic rdict{};

        for(auto&& kv : dict) {
            if(has_symbol(*kv.first, *index) or has_symbol(*kv.second, *index)) {
                rdict.insert(kv);
            } else {
                ldict.insert(kv);
            }
        }

        auto l = Mul::from_dict(coef, std::move(ldict));
        auto r = Mul::from_dict(one, std::move(rdict));

        auto sr = summation_symbolic(r, index, from, to);
        return mul(l, sr);
        // apart(f, index) ???
    }

    if(is_a<Add>(*f)) {
        auto&& fadd = down_cast<const Add&>(*f);
        RCP<const Basic> l, r;
        fadd.as_two_terms(outArg(l), outArg(r));

        auto lrsum = telescopic(l, r, index, from, to);

        if(not lrsum.is_null()) return lrsum;

        auto lsum = summation(l, index, from, to);
        auto rsum = summation(r, index, from, to);

        if(not lsum.is_null() and not rsum.is_null()) {
            auto res = add(lsum, rsum);
            if(not res.is_null() and not is_a<NaN>(*res)) return res;
        }

        // apart(f, index) ???
    }

    if(is_a<Pow>(*f)) {
        auto&& fpow = down_cast<const Pow&>(*f);
        if(eq(*fpow.get_base(), *index)) {
            auto n = fpow.get_exp();
            if(is_a<Integer>(*n)) {
                auto&& ni = down_cast<const Integer&>(*n).as_integer_class();
                if(ni >= 0) {
                    auto n_plus_1 = integer(ni + 1);
                    return expand(
                        div(
                            sub(
                                bernoulli(n_plus_1, add(to, one)),
                                bernoulli(n_plus_1, from)
                            ),
                            n_plus_1
                        )
                    );
                } else {
                    if(is_a<Integer>(*from) and down_cast<const Integer&>(*from).as_int() >= 1) {
                        if(ni == -1) return sub(sym_harmonic(to), sym_harmonic(sub(from, one)));
                        else return sub(sym_harmonic(to, abs(n)), sym_harmonic(sub(from, one), abs(n)));
                    }
                }
            }
        }
    }

    return make_rcp<Sum>(f, index, from, to);
}


RCP<const Basic> summation(const RCP<const Basic>& f,
                           const RCP<const Symbol>& index,
                           const RCP<const Basic>& from,
                           const RCP<const Basic>& to) {
    if(is_a_Number(*f)) {
        auto&& number = down_cast<const Number&>(*f);
        if(number.is_zero()) return zero;
    }

    if(not has_symbol(*f, *index)) {
        return mul(f, add(sub(to, from), one));
    }

    if(eq(*from, *to)) {
        return f->subs({{index, from}});
    }

    if(is_a<Piecewise>(*f)) {
        auto&& pw = down_cast<const Piecewise&>(*f);
        for(auto&& pc : pw.get_vec()) {
            if(has_symbol(*pc.second, *index)) {
                return null;
            }
        }
        PiecewiseVec newVec(pw.get_vec().size());
        for(auto&& pc : pw.get_vec()) {
            newVec.emplace_back(summation(pc.first, index, from, to), pc.second);
        }
        return piecewise(std::move(newVec));
    }

    auto&& dif = sub(to, from);
    auto definite = is_a<Integer>(*dif);

    if(definite and unified_compare(dif, 100) < 0) {
        return summation_direct(f, index, from, to, down_cast<const Integer&>(*dif));
    }

    auto&& value = summation_symbolic(expand(f), index, from, to);
    if(not value.is_null()) return std::move(value);
    return SymEngine::null;
}

bool Sum::__eq__(const Basic &o) const {
    if(not is_a<Sum>(o)) return false;
    auto&& that = down_cast<const Sum &>(o);

    return eq(*f_, *that.f_)
           and eq(*index_, *that.index_)
           and eq(*from_, *that.from_)
           and eq(*to_, *that.to_);
}

int Sum::compare(const Basic &o) const {
    SYMENGINE_ASSERT(is_a<Sum>(o))

    auto&& that = down_cast<const Sum&>(o);
    if(auto step = unified_compare(f_, that.f_)) return step;
    if(auto step = unified_compare(index_, that.index_)) return step;
    if(auto step = unified_compare(from_, that.from_)) return step;
    if(auto step = unified_compare(to_, that.to_)) return step;
    return 0;
}

hash_t Sum::__hash__() const {
    hash_t hash { SUM };
    hash_combine(hash, *f_);
    hash_combine(hash, *index_);
    hash_combine(hash, *from_);
    hash_combine(hash, *to_);
    return hash;
}

} /* namespace SymEngine */
