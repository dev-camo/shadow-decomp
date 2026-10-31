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
//  Adapted from Boost 1.32: the include guard follows this project's naming and the
//  configuration is resolved for the game, which is built without exceptions
//  (BOOST_NO_EXCEPTIONS): boost::throw_exception is only declared, and the game supplies it.
//

#include <exception>

namespace boost {

void throw_exception(std::exception const &e); // user defined

} // namespace boost

#endif
