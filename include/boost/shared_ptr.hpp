#ifndef BOOST_SHARED_PTR_HPP
#define BOOST_SHARED_PTR_HPP
#include "boost/checked_delete.hpp"
#include "layout_check.h"
#include <exception>
#include <new>
#include <typeinfo>
#include <utility>

/* The boost smart pointers the game uses (its RTTI names boost::detail::sp_counted_base and
 * boost::checked_deleter<...>), written in the style of boost 1.32 with the single-threaded
 * reference count. Only the members recovered code uses are present, plus the virtual functions
 * that give sp_counted_base its vtable layout and the count copy operations that keep any copy of a
 * smart pointer correct. Their bodies are visible because the game's inlining depends on them: a
 * function whose body calls one of these inline members is itself never inlined (see
 * cflags_game_cpp in configure.py). */
namespace boost {
template <class T> class weak_ptr;

/* The game is built without exceptions: it supplies its own throw_exception, which returns, and
 * the code after each call carries on. */
void throw_exception(const std::exception &e);

/* The exception for a shared_ptr made from a weak_ptr whose object is gone. */
class bad_weak_ptr : public std::exception {
  public:
    virtual const char *what() const {
        return "boost::bad_weak_ptr";
    }
};
CHECK_SIZEOF(boost::bad_weak_ptr, 0x4);

namespace detail {
class weak_count;

/* The control block shared by the shared_ptr and weak_ptr instances that refer to one object. The
 * object is disposed of when the last shared reference goes, and the block destroys itself when the
 * last weak reference goes; the shared references together hold one weak reference. The virtual
 * functions are declared first, so the counts follow the vtable pointer: weak_count's out-of-line
 * constructor increments the weak count at 0x8, and its destructor calls destroy through vtable
 * offset 0x10, after the destructor and dispose. */
class sp_counted_base {
  public:
    virtual ~sp_counted_base() {}

    /* Destroys the managed object. */
    virtual void dispose() = 0;
    /* Destroys this control block. */
    virtual void destroy() {
        delete this;
    }
    virtual void *get_deleter(const std::type_info &ti) = 0;

    void add_ref_copy() {
        ++use_count_;
    }

    void release() {
        if (--use_count_ == 0) {
            dispose();
            weak_release();
        }
    }

    long use_count() const {
        return use_count_;
    }

    /* Takes a shared reference for a shared_ptr made from a weak_ptr. */
    void add_ref_lock() {
        if (use_count_ == 0) {
            boost::throw_exception(bad_weak_ptr());
        }
        ++use_count_;
    }

    void weak_add_ref() {
        ++weak_count_;
    }

    void weak_release() {
        if (--weak_count_ == 0) {
            destroy();
        }
    }

  private:
    /* 0x4 */ long use_count_;
    /* 0x8 */ long weak_count_;
};
CHECK_SIZEOF(boost::detail::sp_counted_base, 0xC);

/* The control block for an object owned through a pointer of type P and released by a deleter of
 * type D (the game's: boost::checked_deleter<...>). */
template <class P, class D> class sp_counted_base_impl : public sp_counted_base {
  public:
    sp_counted_base_impl(P p, D d) : ptr(p), del(d) {}

    virtual void dispose() {
        del(ptr);
    }

    virtual void *get_deleter(const std::type_info &ti) {
        return ti == typeid(D) ? &del : 0;
    }

  private:
    sp_counted_base_impl(const sp_counted_base_impl &);
    sp_counted_base_impl &operator=(const sp_counted_base_impl &);

    /* 0xC */ P ptr;
    /* 0x10 */ D del;
};

/* The shared reference held by a shared_ptr: copying retains it and destruction releases it. */
class shared_count {
  public:
    shared_count() : pi_(0) {}

    /* The first reference to the object p, released with d. The game is built without exceptions,
     * so a failed allocation of the control block releases the object and reports a bad_alloc
     * through throw_exception, as boost does under BOOST_NO_EXCEPTIONS. */
    template <class P, class D> shared_count(P p, D d) : pi_(0) {
        pi_ = new sp_counted_base_impl<P, D>(p, d);

        if (pi_ == 0) {
            d(p);
            boost::throw_exception(std::bad_alloc());
        }
    }

    /* Takes a shared reference from a weak one; defined after weak_count. */
    shared_count(const weak_count &r);

    shared_count(const shared_count &r) : pi_(r.pi_) {
        if (pi_ != 0) {
            pi_->add_ref_copy();
        }
    }

    ~shared_count() {
        if (pi_ != 0) {
            pi_->release();
        }
    }

    shared_count &operator=(const shared_count &r) {
        sp_counted_base *tmp = r.pi_;

        if (tmp != pi_) {
            if (tmp != 0) {
                tmp->add_ref_copy();
            }
            if (pi_ != 0) {
                pi_->release();
            }
            pi_ = tmp;
        }
        return *this;
    }

    void swap(shared_count &r) {
        sp_counted_base *tmp = r.pi_;

        r.pi_ = pi_;
        pi_ = tmp;
    }

    sp_counted_base *pi_;
};
CHECK_SIZEOF(boost::detail::shared_count, 0x4);

/* Selects the shared_ptr constructor that casts the stored pointer with static_cast. */
struct static_cast_tag {};

/* boost's hook for enable_shared_from_this: the overload for every other type does nothing. It is
 * variadic, so the compiler never expands it in place. */
inline void sp_enable_shared_from_this(const shared_count &, ...) {}

} // namespace detail

/* The copy constructor, assignment and destructor are the compiler-generated ones, which copy,
 * assign and destroy pn, as in boost. */
template <class T> class shared_ptr {
  public:
    typedef shared_ptr<T> this_type;

    shared_ptr() : px(0), pn() {}

    /* Takes ownership of p, to be released with delete. */
    template <class Y> explicit shared_ptr(Y *p) : px(p), pn(p, checked_deleter<Y>()) {
        detail::sp_enable_shared_from_this(pn, p, p);
    }

    /* A shared pointer to the object a weak pointer refers to. */
    template <class Y> explicit shared_ptr(const weak_ptr<Y> &r) : pn(r.pn) {
        px = r.px;
    }

    template <class Y>
    shared_ptr(const shared_ptr<Y> &r, detail::static_cast_tag)
        : px(static_cast<T *>(r.px)), pn(r.pn) {}

    /* Releases the object; the pointer becomes empty. */
    void reset() {
        this_type().swap(*this);
    }

    void swap(shared_ptr<T> &other) {
        std::swap(px, other.px);
        pn.swap(other.pn);
    }

    T *operator->() const {
        return px;
    }

    T *get() const {
        return px;
    }

    /* 0x0 */ T *px;
    /* 0x4 */ detail::shared_count pn;
};

template <class T, class U> inline bool operator==(const shared_ptr<T> &a, const shared_ptr<U> &b) {
    return a.get() == b.get();
}

/* Converts the stored pointer with static_cast, sharing the same reference count. */
template <class T, class U> shared_ptr<T> static_pointer_cast(const shared_ptr<U> &r) {
    return shared_ptr<T>(r, detail::static_cast_tag());
}

} // namespace boost

#endif
