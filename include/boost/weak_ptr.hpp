#ifndef BOOST_WEAK_PTR_HPP
#define BOOST_WEAK_PTR_HPP

#include "boost/shared_ptr.hpp"

/* Declarations of the boost weak pointer members recovered code calls; see shared_ptr.hpp. */
namespace boost {
namespace detail {

/* The weak reference count held by a weak_ptr. Copying retains and destruction releases the weak
 * count, so the special members are declared (compiled elsewhere) rather than left to the compiler. */
class weak_count {
  public:
    weak_count();
    weak_count(const weak_count &r);
    weak_count(const shared_count &r);
    ~weak_count();
    weak_count &operator=(const weak_count &r);

    sp_counted_base *pi_;
};

} // namespace detail

template <class T> class weak_ptr {
  public:
    weak_ptr();
    weak_ptr(const weak_ptr &r);
    /* Each unit that converts a shared_ptr compiles its own instance of this template. */
    template <class Y> weak_ptr(const shared_ptr<Y> &r);
    ~weak_ptr();
    weak_ptr &operator=(const weak_ptr &r);

    /* 0x0 */ T *px;
    /* 0x4 */ detail::weak_count pn;
};

template <class T>
template <class Y>
weak_ptr<T>::weak_ptr(const shared_ptr<Y> &r) : px(r.px), pn(r.pn) {}

} // namespace boost

#endif
