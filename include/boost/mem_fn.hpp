#ifndef BOOST_MEM_FN_HPP
#define BOOST_MEM_FN_HPP

/* The member function adapter that boost::bind wraps a member function pointer in, in the style of
 * boost 1.32 (boost/mem_fn.hpp). Only the adapter for a member function taking one argument is
 * written, because that is the one the game's callbacks use. */
namespace boost {
namespace _mfi {

/* Calls a member function of T taking one argument of type A1 on a T pointer. */
template <class R, class T, class A1> class mf1 {
  public:
    typedef R result_type;
    typedef T *first_argument_type;
    typedef A1 second_argument_type;

    explicit mf1(R (T::*f)(A1)) : f_(f) {}

    R operator()(T *p, A1 a1) const {
        return (p->*f_)(a1);
    }

  private:
    R (T::*f_)(A1);
};

} // namespace _mfi
} // namespace boost

#endif
