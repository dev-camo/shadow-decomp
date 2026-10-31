#ifndef BOOST_SCOPED_PTR_HPP
#define BOOST_SCOPED_PTR_HPP

//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/smart_ptr/scoped_ptr.htm
//
//  Adapted from Boost 1.32 (boost/scoped_ptr.hpp): the include guard follows this project's
//  naming. The debug hooks, the auto_ptr constructor, the conversion to bool and the free swap
//  and get_pointer functions are left out because the reconstructed units use none of them.
//  BOOST_ASSERT is resolved for an NDEBUG build, so the checks in reset(), operator* and
//  operator-> are left out.

#include "boost/checked_delete.hpp"

namespace boost {

//  scoped_ptr mimics a built-in pointer except that it guarantees deletion
//  of the object pointed to, either on destruction of the scoped_ptr or via
//  an explicit reset(). scoped_ptr is a simple solution for simple needs;
//  use shared_ptr or std::auto_ptr if your needs are more complex.

template <class T>
class scoped_ptr // noncopyable
{
  private:
    /* 0x0 */ T *ptr;

    scoped_ptr(scoped_ptr const &);
    scoped_ptr &operator=(scoped_ptr const &);

    typedef scoped_ptr<T> this_type;

  public:
    typedef T element_type;

    explicit scoped_ptr(T *p = 0)
        : ptr(p) // never throws
    {}

    ~scoped_ptr() // never throws
    {
        boost::checked_delete(ptr);
    }

    void reset(T *p = 0) // never throws
    {
        this_type(p).swap(*this);
    }

    T &operator*() const // never throws
    {
        return *ptr;
    }

    T *operator->() const // never throws
    {
        return ptr;
    }

    T *get() const // never throws
    {
        return ptr;
    }

    bool operator!() const // never throws
    {
        return ptr == 0;
    }

    void swap(scoped_ptr &b) // never throws
    {
        T *tmp = b.ptr;
        b.ptr = ptr;
        ptr = tmp;
    }
};

} // namespace boost

#endif
