#ifndef BOOST_MEM_FN_HPP
#define BOOST_MEM_FN_HPP

//
//  mem_fn.hpp - a generalization of std::mem_fun[_ref]
//
//  Copyright (c) 2001, 2002 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2001 David Abrahams
//
//  bind/mem_fn_template.hpp
//
//  Copyright (c) 2001 Peter Dimov and Multi Media Ltd.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/bind/mem_fn.html for documentation.
//
//  Adapted from Boost 1.32 (the include guard follows this project's naming): the mf1 class of
//  boost/bind/mem_fn_template.hpp as instantiated by boost/mem_fn.hpp for a compiler with void
//  returns and a single calling convention. It is the only member function adapter the
//  reconstructed units use. The other arities, the const adapters (the game also uses cmf1, in
//  units not yet reconstructed) and the mem_fn() overloads are left out.
//

#include "boost/get_pointer.hpp"

namespace boost {

namespace _mfi // mem_fun_impl
{

// mf1

template <class R, class T, class A1> class mf1 {
  public:
    typedef R result_type;
    typedef T *first_argument_type;
    typedef A1 second_argument_type;

  private:
    typedef R (T::*F)(A1);
    F f_;

    template <class U, class B1> R call(U &u, T const *, B1 &b1) const {
        return (u.*f_)(b1);
    }

    template <class U, class B1> R call(U &u, void const *, B1 &b1) const {
        return (get_pointer(u)->*f_)(b1);
    }

  public:
    explicit mf1(F f) : f_(f) {}

    R operator()(T *p, A1 a1) const {
        return (p->*f_)(a1);
    }

    template <class U> R operator()(U &u, A1 a1) const {
        return call(u, &u, a1);
    }

    R operator()(T &t, A1 a1) const {
        return (t.*f_)(a1);
    }

    bool operator==(mf1 const &rhs) const {
        return f_ == rhs.f_;
    }

    bool operator!=(mf1 const &rhs) const {
        return f_ != rhs.f_;
    }
};

} // namespace _mfi

} // namespace boost

#endif
