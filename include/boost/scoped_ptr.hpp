#ifndef BOOST_SCOPED_PTR_HPP
#define BOOST_SCOPED_PTR_HPP

#include "boost/checked_delete.hpp"
#include "layout_check.h"

/* The boost pointer that owns one object for its own lifetime, in the style of boost 1.32
 * (boost/scoped_ptr.hpp). Only the members the game's code uses are written. The game calls
 * operator-> out of line where its result is the object of an inline member call, as the comment
 * at MakeTranslationMatrix in Game/fn_8040_vector_types.h describes for an argument of an inline
 * call. */
namespace boost {

template <class T> class scoped_ptr {
  public:
    explicit scoped_ptr(T *p = 0) : ptr(p) {}

    ~scoped_ptr() {
        boost::checked_delete(ptr);
    }

    T &operator*() const {
        return *ptr;
    }

    T *operator->() const {
        return ptr;
    }

    T *get() const {
        return ptr;
    }

  private:
    scoped_ptr(const scoped_ptr &);
    scoped_ptr &operator=(const scoped_ptr &);

    /* 0x0 */ T *ptr;
};

} // namespace boost

#endif
