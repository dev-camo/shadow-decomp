#ifndef BOOST_ASSERT_HPP
#define BOOST_ASSERT_HPP

//
//  boost/assert.hpp - BOOST_ASSERT(expr)
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/utility/assert.html for documentation.
//
//  Adapted from Boost 1.32, whose header has no include guard. This copy has one and keeps
//  only the expansion of a build without assertions: in Boost 1.32 BOOST_ASSERT(expr) is
//  assert(expr) unless BOOST_DISABLE_ASSERTS or BOOST_ENABLE_ASSERT_HANDLER is defined, and
//  assert is ((void)0) under NDEBUG. The assertion handler branch is left out. The expansion
//  still leaves an expression statement in the function that uses it; see
//  boost::shared_ptr::operator-> in shared_ptr.hpp.
//

#define BOOST_ASSERT(expr) ((void)0)

#endif // BOOST_ASSERT_HPP
