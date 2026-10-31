// Boost.Signals library

// Copyright Douglas Gregor 2001-2003. Use, modification and
// distribution is subject to the Boost Software License, Version
// 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// For more information, see http://www.boost.org/libs/signals

// Adapted from Boost 1.32 (boost/visit_each.hpp): the include of boost/config.hpp is left out
// because nothing here depends on it. boost/bind.hpp uses visit_each in its accept members.

#ifndef BOOST_VISIT_EACH_HPP
#define BOOST_VISIT_EACH_HPP

namespace boost {

template <typename Visitor, typename T> inline void visit_each(Visitor &visitor, const T &t, long) {
    visitor(t);
}

template <typename Visitor, typename T> inline void visit_each(Visitor &visitor, const T &t) {
    visit_each(visitor, t, 0);
}

} // namespace boost

#endif // BOOST_VISIT_EACH_HPP
