#ifndef BOOST_ENABLE_SHARED_FROM_THIS_HPP
#define BOOST_ENABLE_SHARED_FROM_THIS_HPP

//
//  enable_shared_from_this.hpp
//
//  Copyright (c) 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/enable_shared_from_this.html for documentation.
//
//  Adapted from Boost 1.32 (the include guard follows this project's naming). BOOST_ASSERT is
//  resolved for an NDEBUG build, so the checks in shared_from_this are left out. The bodies stay
//  in the class as in Boost: the game's inlining depends on them (see cflags_game_cpp in
//  configure.py).
//

#include "boost/shared_ptr.hpp"
#include "boost/weak_ptr.hpp"

namespace boost {

template <class T> class enable_shared_from_this {
  protected:
    enable_shared_from_this() {}

    enable_shared_from_this(enable_shared_from_this const &) {}

    enable_shared_from_this &operator=(enable_shared_from_this const &) {
        return *this;
    }

    ~enable_shared_from_this() {}

  public:
    shared_ptr<T> shared_from_this() {
        shared_ptr<T> p(_internal_weak_this);
        return p;
    }

    shared_ptr<T const> shared_from_this() const {
        shared_ptr<T const> p(_internal_weak_this);
        return p;
    }

    typedef T _internal_element_type; // for bcc 5.5.1
    /* 0x0 */ mutable weak_ptr<_internal_element_type> _internal_weak_this;
};

} // namespace boost

#endif
