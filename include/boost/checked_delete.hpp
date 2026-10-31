#ifndef BOOST_CHECKED_DELETE_HPP
#define BOOST_CHECKED_DELETE_HPP

#include "layout_check.h"

/* The deleters that boost's smart pointers apply to the objects they own, in the style of boost
 * 1.32 (boost/checked_delete.hpp): a delete that refuses an incomplete type, and the function
 * object that shared_ptr's constructor from a raw pointer stores in its count. */
namespace boost {

template <class T> inline void checked_delete(T *x) {
    typedef char type_must_be_complete[sizeof(T) ? 1 : -1];
    (void)sizeof(type_must_be_complete);
    delete x;
}

template <class T> struct checked_deleter {
    typedef void result_type;
    typedef T *argument_type;

    void operator()(T *x) const {
        boost::checked_delete(x);
    }
};

} // namespace boost

#endif
