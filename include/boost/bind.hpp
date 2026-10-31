#ifndef BOOST_BIND_HPP
#define BOOST_BIND_HPP

//
//  bind.hpp - binds function objects to arguments
//
//  Copyright (c) 2001-2004 Peter Dimov and Multi Media Ltd.
//  Copyright (c) 2001 David Abrahams
//
//  bind/bind_template.hpp
//
//  Copyright (c) 2001-2004 Peter Dimov and Multi Media Ltd.
//
//  bind/bind_mf_cc.hpp
//
//  Copyright (c) 2001 Peter Dimov and Multi Media Ltd.
//
//  bind/arg.hpp, bind/placeholders.hpp
//
//  Copyright (c) 2002 Peter Dimov and Multi Media Ltd.
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/bind/bind.html for documentation.
//
//  Adapted from Boost 1.32: the include guard follows this project's naming, and boost/bind.hpp
//  is merged with the pieces of boost/bind/ it includes, resolved for a compiler with void
//  returns and a single calling convention, and cut to what the reconstructed units use
//  (bind(&C::f, this, _1) with a member function of one argument, see boost/mem_fn.hpp): the
//  lists and call operators of up to two arguments, the member function overload of bind() for
//  two arguments, and the placeholders. The overloads for function pointers and const member
//  functions are left out (the game also uses bind(f, _1) with a function pointer and cmf1, in
//  units not yet reconstructed), as are boost::reference_wrapper and data member support. The
//  bodies stay in the classes as in Boost: the game's inlining depends on them (see
//  cflags_game_cpp in configure.py).
//

#include "boost/mem_fn.hpp"
#include "boost/visit_each.hpp"

namespace boost {

template <int I> class arg {};

template <int I> bool operator==(arg<I> const &, arg<I> const &) {
    return true;
}

namespace _bi // implementation details
{

// result_traits

template <class R, class F> struct result_traits {
    typedef R type;
};

struct unspecified {};

template <class F> struct result_traits<unspecified, F> {
    typedef typename F::result_type type;
};

// ref_compare

template <class T> bool ref_compare(T const &a, T const &b, long) {
    return a == b;
}

// bind_t forward declaration for listN

template <class R, class F, class L> class bind_t;

// value

template <class T> class value {
  public:
    value(T const &t) : t_(t) {}

    T &get() {
        return t_;
    }
    T const &get() const {
        return t_;
    }

    bool operator==(value const &rhs) const {
        return t_ == rhs.t_;
    }

  private:
    T t_;
};

// type

template <class T> class type {};

// unwrap

template <class F> inline F &unwrap(F *f, long) {
    return *f;
}

// listN

class list0 {
  public:
    list0() {}

    template <class T> T &operator[](value<T> &v) const {
        return v.get();
    }

    template <class T> T const &operator[](value<T> const &v) const {
        return v.get();
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> const &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class A> R operator()(type<R>, F &f, A &, long) {
        return unwrap(&f, 0)();
    }

    template <class R, class F, class A> R operator()(type<R>, F const &f, A &, long) const {
        return unwrap(&f, 0)();
    }

    template <class F, class A> void operator()(type<void>, F &f, A &, int) {
        unwrap(&f, 0)();
    }

    template <class F, class A> void operator()(type<void>, F const &f, A &, int) const {
        unwrap(&f, 0)();
    }

    template <class V> void accept(V &) const {}

    bool operator==(list0 const &) const {
        return true;
    }
};

template <class A1> class list1 {
  public:
    explicit list1(A1 a1) : a1_(a1) {}

    A1 operator[](boost::arg<1>) const {
        return a1_;
    }

    A1 operator[](boost::arg<1> (*)()) const {
        return a1_;
    }

    template <class T> T &operator[](value<T> &v) const {
        return v.get();
    }

    template <class T> T const &operator[](value<T> const &v) const {
        return v.get();
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> const &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class A> R operator()(type<R>, F &f, A &a, long) {
        return unwrap(&f, 0)(a[a1_]);
    }

    template <class R, class F, class A> R operator()(type<R>, F const &f, A &a, long) const {
        return unwrap(&f, 0)(a[a1_]);
    }

    template <class F, class A> void operator()(type<void>, F &f, A &a, int) {
        unwrap(&f, 0)(a[a1_]);
    }

    template <class F, class A> void operator()(type<void>, F const &f, A &a, int) const {
        unwrap(&f, 0)(a[a1_]);
    }

    template <class V> void accept(V &v) const {
        visit_each(v, a1_, 0);
    }

    bool operator==(list1 const &rhs) const {
        return ref_compare(a1_, rhs.a1_, 0);
    }

  private:
    A1 a1_;
};

template <class A1, class A2> class list2 {
  public:
    list2(A1 a1, A2 a2) : a1_(a1), a2_(a2) {}

    A1 operator[](boost::arg<1>) const {
        return a1_;
    }
    A2 operator[](boost::arg<2>) const {
        return a2_;
    }

    A1 operator[](boost::arg<1> (*)()) const {
        return a1_;
    }
    A2 operator[](boost::arg<2> (*)()) const {
        return a2_;
    }

    template <class T> T &operator[](value<T> &v) const {
        return v.get();
    }

    template <class T> T const &operator[](value<T> const &v) const {
        return v.get();
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class L>
    typename result_traits<R, F>::type operator[](bind_t<R, F, L> const &b) const {
        return b.eval(*this);
    }

    template <class R, class F, class A> R operator()(type<R>, F &f, A &a, long) {
        return unwrap(&f, 0)(a[a1_], a[a2_]);
    }

    template <class R, class F, class A> R operator()(type<R>, F const &f, A &a, long) const {
        return unwrap(&f, 0)(a[a1_], a[a2_]);
    }

    template <class F, class A> void operator()(type<void>, F &f, A &a, int) {
        unwrap(&f, 0)(a[a1_], a[a2_]);
    }

    template <class F, class A> void operator()(type<void>, F const &f, A &a, int) const {
        unwrap(&f, 0)(a[a1_], a[a2_]);
    }

    template <class V> void accept(V &v) const {
        visit_each(v, a1_, 0);
        visit_each(v, a2_, 0);
    }

    bool operator==(list2 const &rhs) const {
        return ref_compare(a1_, rhs.a1_, 0) && ref_compare(a2_, rhs.a2_, 0);
    }

  private:
    A1 a1_;
    A2 a2_;
};

// bind_t

template <class R, class F, class L> class bind_t {
  public:
    typedef bind_t this_type;

    bind_t(F f, L const &l) : f_(f), l_(l) {}

    typedef typename result_traits<R, F>::type result_type;

    result_type operator()() {
        list0 a;
        return l_(type<result_type>(), f_, a, 0);
    }

    result_type operator()() const {
        list0 a;
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A1> result_type operator()(A1 &a1) {
        list1<A1 &> a(a1);
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A1> result_type operator()(A1 &a1) const {
        list1<A1 &> a(a1);
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A1, class A2> result_type operator()(A1 &a1, A2 &a2) {
        list2<A1 &, A2 &> a(a1, a2);
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A1, class A2> result_type operator()(A1 &a1, A2 &a2) const {
        list2<A1 &, A2 &> a(a1, a2);
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A> result_type eval(A &a) {
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class A> result_type eval(A &a) const {
        return l_(type<result_type>(), f_, a, 0);
    }

    template <class V> void accept(V &v) const {
        visit_each(v, f_, 0);
        l_.accept(v);
    }

    bool compare(this_type const &rhs) const {
        return ref_compare(f_, rhs.f_, 0) && l_ == rhs.l_;
    }

  private:
    F f_;
    L l_;
};

// add_value

template <class T> struct add_value {
    typedef value<T> type;
};

template <class T> struct add_value<value<T> > {
    typedef value<T> type;
};

template <int I> struct add_value<arg<I> > {
    typedef boost::arg<I> type;
};

template <int I> struct add_value<arg<I> (*)()> {
    typedef boost::arg<I> (*type)();
};

template <class R, class F, class L> struct add_value<bind_t<R, F, L> > {
    typedef bind_t<R, F, L> type;
};

// list_av_N

template <class A1, class A2> struct list_av_2 {
    typedef typename add_value<A1>::type B1;
    typedef typename add_value<A2>::type B2;
    typedef list2<B1, B2> type;
};

} // namespace _bi

// bind(member function, object, argument)

template <class R, class T, class B1, class A1, class A2>
_bi::bind_t<R, _mfi::mf1<R, T, B1>, typename _bi::list_av_2<A1, A2>::type> bind(R (T::*f)(B1),
                                                                                A1 a1, A2 a2) {
    typedef _mfi::mf1<R, T, B1> F;
    typedef typename _bi::list_av_2<A1, A2>::type list_type;
    return _bi::bind_t<R, F, list_type>(F(f), list_type(a1, a2));
}

} // namespace boost

// The placeholders, in the form bind/placeholders.hpp uses for Metrowerks compilers.
namespace {
static boost::arg<1> _1;
static boost::arg<2> _2;
static boost::arg<3> _3;
static boost::arg<4> _4;
static boost::arg<5> _5;
static boost::arg<6> _6;
static boost::arg<7> _7;
static boost::arg<8> _8;
static boost::arg<9> _9;
} // unnamed namespace

#endif
