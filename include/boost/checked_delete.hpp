#ifndef BOOST_CHECKED_DELETE_HPP
#define BOOST_CHECKED_DELETE_HPP

//
//  boost/checked_delete.hpp
//
//  Copyright (c) 2002, 2003 Peter Dimov
//  Copyright (c) 2003 Daniel Frey
//  Copyright (c) 2003 Howard Hinnant
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/utility/checked_delete.html for documentation.
//
//  Adapted from Boost 1.32: the include guard follows this project's naming, the MSVC-only
//  pragma once is dropped, and checked_array_delete and checked_array_deleter are left out
//  because the reconstructed units use no array deleters. The remaining declarations are unchanged.
//

namespace boost {

// verify that types are complete for increased safety

template <class T> inline void checked_delete(T *x) {
    // intentionally complex - simplification causes regressions
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void)sizeof(type_must_be_complete);
    delete x;
}

template <class T> struct checked_deleter {
    typedef void result_type;
    typedef T *argument_type;

    void operator()(T *x) const {
        // boost:: disables ADL
        boost::checked_delete(x);
    }
};

} // namespace boost

#endif
