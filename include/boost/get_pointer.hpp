#ifndef BOOST_GET_POINTER_HPP
#define BOOST_GET_POINTER_HPP

// Copyright Peter Dimov and David Abrahams 2002.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
// Adapted from Boost 1.32 (boost/get_pointer.hpp): the include guard follows this project's
// naming and the overload for std::auto_ptr is left out (the reconstructed units do not use it).

namespace boost {

// get_pointer(p) extracts a ->* capable pointer from p

template <class T> T *get_pointer(T *p) {
    return p;
}

// get_pointer(shared_ptr<T> const & p) has been moved to shared_ptr.hpp

} // namespace boost

#endif
