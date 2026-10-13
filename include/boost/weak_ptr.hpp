#ifndef BOOST_WEAK_PTR_HPP
#define BOOST_WEAK_PTR_HPP

#include "boost/shared_ptr.hpp"

/* Declarations of the boost weak pointer members recovered code calls; see shared_ptr.hpp. */
namespace boost {
namespace detail {

/* The weak reference count held by a weak_ptr. */
class weak_count {
  public:
    sp_counted_base *pi_;
};

} // namespace detail

template <class T> class weak_ptr {
  public:
    weak_ptr();
    ~weak_ptr();

    /* 0x0 */ T *px;
    /* 0x4 */ detail::weak_count pn;
};

} // namespace boost

#endif
