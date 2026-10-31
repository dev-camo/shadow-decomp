#ifndef BOOST_BIND_HPP
#define BOOST_BIND_HPP

#include "boost/mem_fn.hpp"
#include "layout_check.h"

/* boost::bind for a member function with one argument, called with the object pointer bound and
 * the first call argument forwarded (bind(&C::f, this, _1)), in the style of boost 1.32. Only
 * what that expression needs is written: the bound pointer (value), the placeholder _1, the two
 * argument lists, bind_t, and the bind overload for member functions. The bodies are visible
 * because the game's inlining depends on them (see shared_ptr.hpp). */
namespace boost {

/* A placeholder for the I-th argument of the call. */
template <int I> class arg {};

namespace _bi {

template <class T> class value {
  public:
    value(const T &t) : t_(t) {}

    T &get() {
        return t_;
    }

    const T &get() const {
        return t_;
    }

  private:
    T t_;
};

/* Selects the overload of a bound call that returns void. */
template <class T> class type {};

template <class F> inline F &unwrap(F *f, long) {
    return *f;
}

/* The arguments of a call, as references. */
template <class A1> class list1 {
  public:
    explicit list1(A1 a1) : a1_(a1) {}

    A1 operator[](boost::arg<1>) const {
        return a1_;
    }

    template <class T> T &operator[](value<T> &v) const {
        return v.get();
    }

  private:
    A1 a1_;
};

/* The stored arguments of a bind expression. Boost also has a version of operator() for calls that
 * return a value; only the void one is written. */
template <class A1, class A2> class list2 {
  public:
    list2(A1 a1, A2 a2) : a1_(a1), a2_(a2) {}

    template <class T> T &operator[](value<T> &v) const {
        return v.get();
    }

    template <class F, class A> void operator()(type<void>, F &f, A &a, int) {
        unwrap(&f, 0)(a[a1_], a[a2_]);
    }

  private:
    A1 a1_;
    A2 a2_;
};

/* A callable that binds the stored arguments L to F. boost's operator() returns R; here R is void
 * and the call is written for that. */
template <class R, class F, class L> class bind_t {
  public:
    bind_t(F f, const L &l) : f_(f), l_(l) {}

    template <class A1> void operator()(A1 &a1) {
        list1<A1 &> a(a1);
        l_(type<void>(), f_, a, 0);
    }

  private:
    F f_;
    L l_;
};

/* Wraps a bound value that is not itself an argument or a nested bind. */
template <class T> struct add_value {
    typedef value<T> type;
};

template <int I> struct add_value<boost::arg<I> > {
    typedef boost::arg<I> type;
};

template <class A1, class A2> struct list_av_2 {
    typedef typename add_value<A1>::type B1;
    typedef typename add_value<A2>::type B2;
    typedef list2<B1, B2> type;
};

} // namespace _bi

template <class R, class T, class B1, class A1, class A2>
_bi::bind_t<R, _mfi::mf1<R, T, B1>, typename _bi::list_av_2<A1, A2>::type> bind(R (T::*f)(B1),
                                                                                A1 a1, A2 a2) {
    typedef _mfi::mf1<R, T, B1> F;
    typedef typename _bi::list_av_2<A1, A2>::type list_type;
    return _bi::bind_t<R, F, list_type>(F(f), list_type(a1, a2));
}

} // namespace boost

/* The placeholder is a static object in an unnamed namespace, as boost defines it for this
 * compiler; a unit that uses it gets its own copy in .sbss. */
namespace {
static boost::arg<1> _1;
}

#endif
