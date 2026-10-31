#ifndef BOOST_UTILITY_ADDRESSOF_HPP
#define BOOST_UTILITY_ADDRESSOF_HPP

/* The address of an object (boost 1.32 writes it with casts through char & so that an overloaded
 * operator& is bypassed; none of the types the game takes the address of overloads it). */
namespace boost {

template <class T> T *addressof(T &v) {
    return &v;
}

} // namespace boost

#endif
