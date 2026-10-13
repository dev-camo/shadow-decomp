#ifndef BOOST_SHARED_PTR_HPP
#define BOOST_SHARED_PTR_HPP

/* The boost smart pointers the game uses (its RTTI names boost::detail::sp_counted_base and
 * boost::checked_deleter<...>). These are declarations only, of the members recovered code calls:
 * the game's copies of these template members are out-of-line instances compiled elsewhere. */
namespace boost {
namespace detail {

class sp_counted_base;

/* The reference count owned by a shared_ptr. Copying retains and destruction releases the count,
 * so the special members are declared (compiled elsewhere) rather than left to the compiler. */
class shared_count {
  public:
    shared_count();
    shared_count(const shared_count &r);
    ~shared_count();
    shared_count &operator=(const shared_count &r);

    sp_counted_base *pi_;
};

} // namespace detail

template <class T> class shared_ptr {
  public:
    shared_ptr();
    shared_ptr(const shared_ptr &r);
    ~shared_ptr();
    shared_ptr &operator=(const shared_ptr &r);

    /* 0x0 */ T *px;
    /* 0x4 */ detail::shared_count pn;
};

} // namespace boost

#endif
