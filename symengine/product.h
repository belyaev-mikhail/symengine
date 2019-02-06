#ifndef SYMENGINE_PRODUCT_H
#define SYMENGINE_PRODUCT_H

#include <symengine/basic.h>
#include <symengine/symbol.h>

namespace SymEngine
{

class Prod : public Basic {
private:
    RCP<const Basic> f_;
    RCP<const Symbol> index_;
    RCP<const Basic> from_;
    RCP<const Basic> to_;

public:
    IMPLEMENT_TYPEID(PROD)

    Prod(const RCP<const Basic>& f,
        const RCP<const Symbol>& index,
        const RCP<const Basic>& from,
        const RCP<const Basic>& to):
        f_{f}, index_{index}, from_{from}, to_{to} {
        SYMENGINE_ASSIGN_TYPEID()
        SYMENGINE_ASSERT(is_canonical(f, index, from, to))
    }

    virtual hash_t __hash__() const override;
    virtual bool __eq__(const Basic &o) const override;
    virtual int compare(const Basic &o) const override;

    bool is_canonical(const RCP<const Basic>& f,
                      const RCP<const Basic>& index,
                      const RCP<const Basic>& from,
                      const RCP<const Basic>& to) const {
        return true; //rewrite
    }

    virtual vec_basic get_args() const override {
        return { f_, index_, from_, to_ };
    }

    inline const RCP<const Basic>& get_function() const noexcept { return f_; }
    inline const RCP<const Symbol>& get_index() const noexcept { return index_; }
    inline const RCP<const Basic>& get_from() const noexcept { return from_; }
    inline const RCP<const Basic>& get_to() const noexcept { return to_; }

};

RCP<const Basic> product(const RCP<const Basic>& f,
                         const RCP<const Symbol>& index,
                         const RCP<const Basic>& from,
                         const RCP<const Basic>& to);

} // SymEngine


#endif //SYMENGINE_PRODUCT_H
