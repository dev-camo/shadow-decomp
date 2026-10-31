#ifndef BOOST_UTILITY_ADDRESSOF_HPP
#define BOOST_UTILITY_ADDRESSOF_HPP

// Copyright (C) 2002 Brad King (brad.king@kitware.com)
//                    Douglas Gregor (gregod@cs.rpi.edu)
//                    Peter Dimov
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Adapted from Boost 1.32 (boost/utility/addressof.hpp): the include guard follows this
// project's naming and the MSVC 7.0 workaround is dropped; addressof is otherwise unchanged.

namespace boost {

// Do not make addressof() inline. Breaks MSVC 7. (Peter Dimov)

template <typename T> T *addressof(T &v) {
    return reinterpret_cast<T *>(&const_cast<char &>(reinterpret_cast<const volatile char &>(v)));
}

} // namespace boost

#endif
