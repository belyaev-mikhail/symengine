#include <symengine/add.h>
#include <symengine/pow.h>
#include <symengine/complex.h>

#include <symengine/product.h>
#include <symengine/summation.h>
#include <symengine/visitor.h>

namespace SymEngine
{

bool Prod::__eq__(const Basic &o) const {
    if(not is_a<Prod>(o)) return false;
    auto&& that = down_cast<const Prod &>(o);

    return eq(*f_, *that.f_)
           and eq(*index_, *that.index_)
           and eq(*from_, *that.from_)
           and eq(*to_, *that.to_);
}

int Prod::compare(const Basic &o) const {
    SYMENGINE_ASSERT(is_a<Prod>(o))

    auto&& that = down_cast<const Prod&>(o);
    if(auto step = unified_compare(f_, that.f_)) return step;
    if(auto step = unified_compare(index_, that.index_)) return step;
    if(auto step = unified_compare(from_, that.from_)) return step;
    if(auto step = unified_compare(to_, that.to_)) return step;
    return 0;
}

hash_t Prod::__hash__() const {
    hash_t hash { PROD };
    hash_combine(hash, *f_);
    hash_combine(hash, *index_);
    hash_combine(hash, *from_);
    hash_combine(hash, *to_);
    return hash;
}


RCP<const Basic> product(const RCP<const Basic> &f,
                         const RCP<const Symbol> &index,
                         const RCP<const Basic> &from,
                         const RCP<const Basic> &to) {
    if(eq(*f, *one)) return one;
    if(not has_symbol(*f, *index)) {
        return pow(f, add(sub(to, from), one));
    }
    if(eq(*from, *to)) return f->subs({{index, from}});

    auto dif = sub(to, from);
    if(is_a<Integer>(*dif)) {
        auto idif = down_cast<const Integer&>(*dif).as_integer_class();
        vec_basic factors {};
        for(integer_class i = 0; i < idif + 1; ++i) {
            factors.push_back(f->subs({{index, add(from, integer(i))}}));
        }
        return mul(factors);
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
            newVec.emplace_back(product(pc.first, index, from, to), pc.second);
        }
        return piecewise(std::move(newVec));
    }

    // polynoms???


    // TODO: Add & Sum

    if(is_a<Mul>(*f)) {
        auto&& fmul = down_cast<const Mul&>(*f);
        RCP<const Basic> l, r;
        fmul.as_two_terms(outArg(l), outArg(r));

        auto lprod = product(l, index, from, to);
        auto rprod = product(r, index, from, to);

        if(not lprod.is_null() and not rprod.is_null()) {
            auto res = mul(lprod, rprod);
            if(not res.is_null() and not is_a<NaN>(*res)) return res;
        }
    }

    if(is_a<Pow>(*f)) {
        auto&& fpow = down_cast<const Pow&>(*f);

        if(not has_symbol(*fpow.get_base(), *index)) {
            return pow(fpow.get_base(), summation(fpow.get_exp(), index, from, to));
        } else if(not has_symbol(*fpow.get_exp(), *index)) {
            auto recur = product(fpow.get_base(), index, from, to);
            if(not recur.is_null()) {
                return pow(recur, fpow.get_exp());
            }
        }
    }

    return make_rcp<Prod>(f, index, from, to);
}

} /* namespace SymEngine */
