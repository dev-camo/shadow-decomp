#ifndef BOOST_SCOPED_ARRAY_HPP
#define BOOST_SCOPED_ARRAY_HPP

//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/smart_ptr/scoped_array.htm
//

// Adapted from Boost 1.32. Only the interface used by the reconstructed units is
// retained; debug hooks and NDEBUG-disabled assertions are omitted.

#include "boost/checked_delete.hpp"
#include <cstddef>

namespace boost {

template <class T> class scoped_array {
  private:
    T *ptr;

    scoped_array(scoped_array const &);
    scoped_array &operator=(scoped_array const &);

    typedef scoped_array<T> this_type;

  public:
    typedef T element_type;

    explicit scoped_array(T *p = 0) : ptr(p) {}

    ~scoped_array() {
        boost::checked_array_delete(ptr);
    }

    void reset(T *p = 0) {
        this_type(p).swap(*this);
    }

    T &operator[](std::ptrdiff_t i) const {
        return ptr[i];
    }

    T *get() const {
        return ptr;
    }

    void swap(scoped_array &b) {
        T *tmp = b.ptr;
        b.ptr = ptr;
        ptr = tmp;
    }
};

} // namespace boost

#endif
