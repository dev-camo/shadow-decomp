#ifndef BOOST_WEAK_PTR_HPP
#define BOOST_WEAK_PTR_HPP

#include "boost/shared_ptr.hpp"
#include "layout_check.h"

/* The boost weak pointer the game uses, in the same style as shared_ptr.hpp. */
namespace boost {
namespace detail {

/* The weak reference held by a weak_ptr: copying retains it and destruction releases it. */
class weak_count {
  public:
    weak_count() : pi_(0) {}

    weak_count(const shared_count &r) : pi_(r.pi_) {
        if (pi_ != 0) {
            pi_->weak_add_ref();
        }
    }

    weak_count(const weak_count &r) : pi_(r.pi_) {
        if (pi_ != 0) {
            pi_->weak_add_ref();
        }
    }

    ~weak_count() {
        if (pi_ != 0) {
            pi_->weak_release();
        }
    }

    weak_count &operator=(const weak_count &r) {
        sp_counted_base *tmp = r.pi_;

        if (tmp != 0) {
            tmp->weak_add_ref();
        }
        if (pi_ != 0) {
            pi_->weak_release();
        }
        pi_ = tmp;
        return *this;
    }

    sp_counted_base *pi_;
};
CHECK_SIZEOF(boost::detail::weak_count, 0x4);

} // namespace detail

/* The copy constructor, assignment and destructor are the compiler-generated ones, as in boost. */
template <class T> class weak_ptr {
  public:
    weak_ptr() : px(0), pn() {}

    template <class Y> weak_ptr(const shared_ptr<Y> &r) : px(r.px), pn(r.pn) {}

    /* A shared pointer to the object, or an empty one once the object has been released. Declared
     * only: its body has not been reconstructed yet. */
    shared_ptr<T> lock() const;

    /* 0x0 */ T *px;
    /* 0x4 */ detail::weak_count pn;
};

} // namespace boost

#endif
