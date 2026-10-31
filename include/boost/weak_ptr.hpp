#ifndef BOOST_WEAK_PTR_HPP
#define BOOST_WEAK_PTR_HPP

//
//  weak_ptr.hpp
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov
//
//  detail/shared_count.hpp
//
//  Copyright (c) 2001, 2002, 2003 Peter Dimov and Multi Media Ltd.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/weak_ptr.htm for documentation.
//
//  Adapted from Boost 1.32 (the include guard follows this project's naming): boost/weak_ptr.hpp
//  and the weak_count class of boost/detail/shared_count.hpp (the shared_count constructor from a
//  weak_count is defined here, after weak_count, instead of in the same file as shared_count).
//  The configuration is resolved as in boost/shared_ptr.hpp, which has the details.
//

#include "boost/shared_ptr.hpp"
#include <utility> // std::swap (this project's MSL headers declare it here, not in <algorithm>)
#include "layout_check.h"

namespace boost {

namespace detail {

class weak_count {
  private:
    /* 0x0 */ sp_counted_base *pi_;

    friend class shared_count;

  public:
    weak_count()
        : pi_(0) // nothrow
    {}

    weak_count(shared_count const &r)
        : pi_(r.pi_) // nothrow
    {
        if (pi_ != 0)
            pi_->weak_add_ref();
    }

    weak_count(weak_count const &r)
        : pi_(r.pi_) // nothrow
    {
        if (pi_ != 0)
            pi_->weak_add_ref();
    }

    ~weak_count() // nothrow
    {
        if (pi_ != 0)
            pi_->weak_release();
    }

    weak_count &operator=(shared_count const &r) // nothrow
    {
        sp_counted_base *tmp = r.pi_;
        if (tmp != 0)
            tmp->weak_add_ref();
        if (pi_ != 0)
            pi_->weak_release();
        pi_ = tmp;

        return *this;
    }

    weak_count &operator=(weak_count const &r) // nothrow
    {
        sp_counted_base *tmp = r.pi_;
        if (tmp != 0)
            tmp->weak_add_ref();
        if (pi_ != 0)
            pi_->weak_release();
        pi_ = tmp;

        return *this;
    }

    void swap(weak_count &r) // nothrow
    {
        sp_counted_base *tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_ != 0 ? pi_->use_count() : 0;
    }

    friend inline bool operator==(weak_count const &a, weak_count const &b) {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(weak_count const &a, weak_count const &b) {
        return std::less<sp_counted_base *>()(a.pi_, b.pi_);
    }
};
CHECK_SIZEOF(boost::detail::weak_count, 0x4);

inline shared_count::shared_count(weak_count const &r) : pi_(r.pi_) {
    if (pi_ != 0) {
        pi_->add_ref_lock();
    } else {
        boost::throw_exception(boost::bad_weak_ptr());
    }
}

} // namespace detail

template <class T> class weak_ptr {
  private:
    typedef weak_ptr<T> this_type;

  public:
    typedef T element_type;

    weak_ptr()
        : px(0), pn() // never throws in 1.30+
    {}

    //  generated copy constructor, assignment, destructor are fine
    //
    //  The "obvious" converting constructor implementation:
    //
    //  template<class Y>
    //  weak_ptr(weak_ptr<Y> const & r): px(r.px), pn(r.pn) // never throws
    //  {
    //  }
    //
    //  has a serious problem.
    //
    //  r.px may already have been invalidated. The px(r.px)
    //  conversion may require access to *r.px (virtual inheritance).
    //
    //  It is not possible to avoid spurious access violations since
    //  in multithreaded programs r.px may be invalidated at any point.
    //

    template <class Y>
    weak_ptr(weak_ptr<Y> const &r)
        : pn(r.pn) // never throws
    {
        px = r.lock().get();
    }

    template <class Y>
    weak_ptr(shared_ptr<Y> const &r)
        : px(r.px), pn(r.pn) // never throws
    {}

    template <class Y>
    weak_ptr &operator=(weak_ptr<Y> const &r) // never throws
    {
        px = r.lock().get();
        pn = r.pn;
        return *this;
    }

    template <class Y>
    weak_ptr &operator=(shared_ptr<Y> const &r) // never throws
    {
        px = r.px;
        pn = r.pn;
        return *this;
    }

    shared_ptr<T> lock() const // never throws
    {
        return expired() ? shared_ptr<element_type>() : shared_ptr<element_type>(*this);
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    bool expired() const // never throws
    {
        return pn.use_count() == 0;
    }

    void reset() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    void swap(this_type &other) // never throws
    {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    void _internal_assign(T *px2, detail::shared_count const &pn2) {
        px = px2;
        pn = pn2;
    }

    template <class Y> bool _internal_less(weak_ptr<Y> const &rhs) const {
        return pn < rhs.pn;
    }

    // Tasteless as this may seem, making all members public allows member templates
    // to work in the absence of member template friends. (Matthew Langston)
    // MWCC does not support member template friends (BOOST_NO_MEMBER_TEMPLATE_FRIENDS), so
    // this is the form boost selects for it.

    /* 0x0 */ T *px;                 // contained pointer
    /* 0x4 */ detail::weak_count pn; // reference counter

}; // weak_ptr

template <class T, class U> inline bool operator<(weak_ptr<T> const &a, weak_ptr<U> const &b) {
    return a._internal_less(b);
}

template <class T> void swap(weak_ptr<T> &a, weak_ptr<T> &b) {
    a.swap(b);
}

// deprecated, provided for backward compatibility
template <class T> shared_ptr<T> make_shared(weak_ptr<T> const &r) {
    return r.lock();
}

} // namespace boost

#endif
