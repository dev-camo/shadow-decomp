#ifndef BOOST_SHARED_PTR_HPP
#define BOOST_SHARED_PTR_HPP

//
//  shared_ptr.hpp
//
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
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
//  See http://www.boost.org/libs/smart_ptr/shared_ptr.htm for documentation.
//
//  Adapted from Boost 1.32 (the include guard follows this project's naming):
//  boost/shared_ptr.hpp and boost/detail/shared_count.hpp (the sp_counted_base,
//  sp_counted_base_impl and shared_count classes, merged into this header). Exception support
//  follows boost/config.hpp. Threads, debug hooks and allocator overrides remain disabled
//  (no BOOST_HAS_THREADS, no BOOST_SP_ENABLE_DEBUG_HOOKS, no allocator override);
//  The selected MWCC Boost configuration also defines
//  BOOST_NO_MEMBER_TEMPLATE_FRIENDS. BOOST_ASSERT is resolved for an NDEBUG build, so the checks
//  in reset(), operator* and operator-> are left out. The workarounds for other compilers are
//  dropped. Left out, because the reconstructed units use none of them: the auto_ptr constructor
//  and assignment, the conversion to bool and operator!, the polymorphic_cast_tag constructor,
//  the deprecated shared_*_cast functions, operator<< and the free get_deleter function. The
//  bodies stay in the classes as in Boost: the game's inlining depends on them (see
//  cflags_game_cpp in configure.py). weak_count and weak_ptr are in boost/weak_ptr.hpp.
//

#include "boost/config.hpp"
#include "boost/checked_delete.hpp"
#include "boost/throw_exception.hpp"
#include <exception>
#include <functional>
#include <new>
#include <typeinfo>
#include <utility> // std::swap (this project's MSL headers declare it here, not in <algorithm>)
#include "layout_check.h"

namespace boost {

template <class T> class weak_ptr;
template <class T> class enable_shared_from_this;

class bad_weak_ptr : public std::exception {
  public:
    virtual char const *what() const throw() {
        return "boost::bad_weak_ptr";
    }
};
CHECK_SIZEOF(boost::bad_weak_ptr, 0x4);

namespace detail {

class sp_counted_base {
  public:
    sp_counted_base() : use_count_(1), weak_count_(1) {}

    virtual ~sp_counted_base() // nothrow
    {}

    // dispose() is called when use_count_ drops to zero, to release
    // the resources managed by *this.

    virtual void dispose() = 0; // nothrow

    // destruct() is called when weak_count_ drops to zero.

    virtual void destruct() // nothrow
    {
        delete this;
    }

    virtual void *get_deleter(std::type_info const &ti) = 0;

    void add_ref_copy() {
        ++use_count_;
    }

    void add_ref_lock() {
        if (use_count_ == 0)
            boost::throw_exception(boost::bad_weak_ptr());
        ++use_count_;
    }

    void release() // nothrow
    {
        // In threaded builds this scope holds the mutex lock.
        {
            long new_use_count = --use_count_;

            if (new_use_count != 0)
                return;
        }

        dispose();
        weak_release();
    }

    void weak_add_ref() // nothrow
    {
        ++weak_count_;
    }

    void weak_release() // nothrow
    {
        long new_weak_count;

        // In threaded builds this scope holds the mutex lock.
        {
            new_weak_count = --weak_count_;
        }

        if (new_weak_count == 0) {
            destruct();
        }
    }

    long use_count() const // nothrow
    {
        return use_count_;
    }

  private:
    sp_counted_base(sp_counted_base const &);
    sp_counted_base &operator=(sp_counted_base const &);

    /* 0x4 */ long use_count_;  // #shared
    /* 0x8 */ long weak_count_; // #weak + (#shared != 0)
};
CHECK_SIZEOF(boost::detail::sp_counted_base, 0xC);

template <class P, class D> class sp_counted_base_impl : public sp_counted_base {
  private:
    /* 0xC */ P ptr;  // copy constructor must not throw
    /* 0x10 */ D del; // copy constructor must not throw

    sp_counted_base_impl(sp_counted_base_impl const &);
    sp_counted_base_impl &operator=(sp_counted_base_impl const &);

    typedef sp_counted_base_impl<P, D> this_type;

  public:
    // pre: initial_use_count <= initial_weak_count, d(p) must not throw

    sp_counted_base_impl(P p, D d) : ptr(p), del(d) {}

    virtual void dispose() // nothrow
    {
        del(ptr);
    }

    virtual void *get_deleter(std::type_info const &ti) {
        return ti == typeid(D) ? &del : 0;
    }
};

class weak_count;

class shared_count {
  private:
    /* 0x0 */ sp_counted_base *pi_;

    friend class weak_count;

  public:
    shared_count()
        : pi_(0) // nothrow
    {}

    template <class P, class D> shared_count(P p, D d) : pi_(0) {
#ifndef BOOST_NO_EXCEPTIONS
        try {
            pi_ = new sp_counted_base_impl<P, D>(p, d);
        } catch (...) {
            d(p); // delete p
            throw;
        }
#else
        pi_ = new sp_counted_base_impl<P, D>(p, d);

        if (pi_ == 0) {
            d(p); // delete p
            boost::throw_exception(std::bad_alloc());
        }
#endif
    }

    ~shared_count() // nothrow
    {
        if (pi_ != 0)
            pi_->release();
    }

    shared_count(shared_count const &r)
        : pi_(r.pi_) // nothrow
    {
        if (pi_ != 0)
            pi_->add_ref_copy();
    }

    explicit shared_count(weak_count const &r); // throws bad_weak_ptr when r.use_count() == 0

    shared_count &operator=(shared_count const &r) // nothrow
    {
        sp_counted_base *tmp = r.pi_;

        if (tmp != pi_) {
            if (tmp != 0)
                tmp->add_ref_copy();
            if (pi_ != 0)
                pi_->release();
            pi_ = tmp;
        }

        return *this;
    }

    void swap(shared_count &r) // nothrow
    {
        sp_counted_base *tmp = r.pi_;
        r.pi_ = pi_;
        pi_ = tmp;
    }

    long use_count() const // nothrow
    {
        return pi_ != 0 ? pi_->use_count() : 0;
    }

    bool unique() const // nothrow
    {
        return use_count() == 1;
    }

    friend inline bool operator==(shared_count const &a, shared_count const &b) {
        return a.pi_ == b.pi_;
    }

    friend inline bool operator<(shared_count const &a, shared_count const &b) {
        return std::less<sp_counted_base *>()(a.pi_, b.pi_);
    }

    void *get_deleter(std::type_info const &ti) const {
        return pi_ ? pi_->get_deleter(ti) : 0;
    }
};
CHECK_SIZEOF(boost::detail::shared_count, 0x4);

struct static_cast_tag {};
struct const_cast_tag {};
struct dynamic_cast_tag {};

template <class T> struct shared_ptr_traits {
    typedef T &reference;
};

template <> struct shared_ptr_traits<void> {
    typedef void reference;
};

template <> struct shared_ptr_traits<void const> {
    typedef void reference;
};

template <> struct shared_ptr_traits<void volatile> {
    typedef void reference;
};

template <> struct shared_ptr_traits<void const volatile> {
    typedef void reference;
};

// enable_shared_from_this support

template <class T, class Y>
void sp_enable_shared_from_this(shared_count const &pn, boost::enable_shared_from_this<T> const *pe,
                                Y const *px) {
    if (pe != 0)
        pe->_internal_weak_this._internal_assign(const_cast<Y *>(px), pn);
}

inline void sp_enable_shared_from_this(shared_count const & /*pn*/, ...) {}

} // namespace detail

//
//  shared_ptr
//
//  An enhanced relative of scoped_ptr with reference counted copy semantics.
//  The object pointed to is deleted when the last shared_ptr pointing to it
//  is destroyed or reset.
//

template <class T> class shared_ptr {
  private:
    typedef shared_ptr<T> this_type;

  public:
    typedef T element_type;
    typedef T value_type;
    typedef T *pointer;
    typedef typename detail::shared_ptr_traits<T>::reference reference;

    shared_ptr()
        : px(0), pn() // never throws in 1.30+
    {}

    template <class Y>
    explicit shared_ptr(Y *p)
        : px(p), pn(p, checked_deleter<Y>()) // Y must be complete
    {
        detail::sp_enable_shared_from_this(pn, p, p);
    }

    //
    // Requirements: D's copy constructor must not throw
    //
    // shared_ptr will release p by calling d(p)
    //

    template <class Y, class D> shared_ptr(Y *p, D d) : px(p), pn(p, d) {
        detail::sp_enable_shared_from_this(pn, p, p);
    }

    //  generated copy constructor, assignment, destructor are fine...

    template <class Y>
    explicit shared_ptr(weak_ptr<Y> const &r)
        : pn(r.pn) // may throw
    {
        // it is now safe to copy r.px, as pn(r.pn) did not throw
        px = r.px;
    }

    template <class Y>
    shared_ptr(shared_ptr<Y> const &r)
        : px(r.px), pn(r.pn) // never throws
    {}

    template <class Y>
    shared_ptr(shared_ptr<Y> const &r, detail::static_cast_tag)
        : px(static_cast<element_type *>(r.px)), pn(r.pn) {}

    template <class Y>
    shared_ptr(shared_ptr<Y> const &r, detail::const_cast_tag)
        : px(const_cast<element_type *>(r.px)), pn(r.pn) {}

    template <class Y>
    shared_ptr(shared_ptr<Y> const &r, detail::dynamic_cast_tag)
        : px(dynamic_cast<element_type *>(r.px)), pn(r.pn) {
        if (px == 0) // need to allocate new counter -- the cast failed
        {
            pn = detail::shared_count();
        }
    }

    template <class Y>
    shared_ptr &operator=(shared_ptr<Y> const &r) // never throws
    {
        px = r.px;
        pn = r.pn; // shared_count::op= doesn't throw
        return *this;
    }

    void reset() // never throws in 1.30+
    {
        this_type().swap(*this);
    }

    template <class Y>
    void reset(Y *p) // Y must be complete
    {
        this_type(p).swap(*this);
    }

    template <class Y, class D> void reset(Y *p, D d) {
        this_type(p, d).swap(*this);
    }

    reference operator*() const // never throws
    {
        return *px;
    }

    T *operator->() const // never throws
    {
        return px;
    }

    T *get() const // never throws
    {
        return px;
    }

    bool unique() const // never throws
    {
        return pn.unique();
    }

    long use_count() const // never throws
    {
        return pn.use_count();
    }

    void swap(shared_ptr<T> &other) // never throws
    {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    template <class Y> bool _internal_less(shared_ptr<Y> const &rhs) const {
        return pn < rhs.pn;
    }

    void *_internal_get_deleter(std::type_info const &ti) const {
        return pn.get_deleter(ti);
    }

    // Tasteless as this may seem, making all members public allows member templates
    // to work in the absence of member template friends. (Matthew Langston)
    // MWCC does not support member template friends (BOOST_NO_MEMBER_TEMPLATE_FRIENDS), so
    // this is the form boost selects for it.

    /* 0x0 */ T *px;                   // contained pointer
    /* 0x4 */ detail::shared_count pn; // reference counter

}; // shared_ptr

template <class T, class U> inline bool operator==(shared_ptr<T> const &a, shared_ptr<U> const &b) {
    return a.get() == b.get();
}

template <class T, class U> inline bool operator!=(shared_ptr<T> const &a, shared_ptr<U> const &b) {
    return a.get() != b.get();
}

template <class T, class U> inline bool operator<(shared_ptr<T> const &a, shared_ptr<U> const &b) {
    return a._internal_less(b);
}

template <class T> inline void swap(shared_ptr<T> &a, shared_ptr<T> &b) {
    a.swap(b);
}

template <class T, class U> shared_ptr<T> static_pointer_cast(shared_ptr<U> const &r) {
    return shared_ptr<T>(r, detail::static_cast_tag());
}

template <class T, class U> shared_ptr<T> const_pointer_cast(shared_ptr<U> const &r) {
    return shared_ptr<T>(r, detail::const_cast_tag());
}

template <class T, class U> shared_ptr<T> dynamic_pointer_cast(shared_ptr<U> const &r) {
    return shared_ptr<T>(r, detail::dynamic_cast_tag());
}

// get_pointer() enables boost::mem_fn to recognize shared_ptr

template <class T> inline T *get_pointer(shared_ptr<T> const &p) {
    return p.get();
}

} // namespace boost

#endif
