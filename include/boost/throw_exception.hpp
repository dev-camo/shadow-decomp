#ifndef BOOST_THROW_EXCEPTION_HPP
#define BOOST_THROW_EXCEPTION_HPP

//
//  boost/throw_exception.hpp
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  http://www.boost.org/libs/utility/throw_exception.html
//
//  Adapted from Boost 1.32: the include guard follows this project's naming.
//  Exception support follows boost/config.hpp. Without exceptions the game supplies
//  the std::exception overload; with exceptions the template throws its argument.
//

#include "boost/config.hpp"

#ifdef BOOST_NO_EXCEPTIONS
#include <exception>
#endif

namespace boost {

#ifdef BOOST_NO_EXCEPTIONS

void throw_exception(std::exception const &e); // user defined

#else

template <class E> inline void throw_exception(E const &e) {
    throw e;
}

#endif

} // namespace boost

#endif
