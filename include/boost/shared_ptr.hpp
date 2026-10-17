#ifndef BOOST_SHARED_PTR_HPP
#define BOOST_SHARED_PTR_HPP
#include "layout_check.h"

/* The boost smart pointers the game uses (its RTTI names boost::detail::sp_counted_base and
 * boost::checked_deleter<...>), written in the style of boost 1.32 with the single-threaded
 * reference count. Only the members recovered code uses are present, plus the virtual functions
 * that give sp_counted_base its vtable layout and the count copy operations that keep any copy of a
 * smart pointer correct. Their bodies are visible because the game's inlining depends on them: a
 * function whose body calls one of these inline members is itself never inlined (see
 * cflags_game_cpp in configure.py). */
namespace std {
class type_info;
}

namespace boost {
namespace detail {

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

/* The shared reference held by a shared_ptr: copying retains it and destruction releases it. */
class shared_count {
  public:
    shared_count() : pi_(0) {}

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

    sp_counted_base *pi_;
};
CHECK_SIZEOF(boost::detail::shared_count, 0x4);

} // namespace detail

/* The copy constructor, assignment and destructor are the compiler-generated ones, which copy,
 * assign and destroy pn, as in boost. */
template <class T> class shared_ptr {
  public:
    shared_ptr() : px(0), pn() {}

    T *operator->() const {
        return px;
    }

    T *get() const {
        return px;
    }

    /* 0x0 */ T *px;
    /* 0x4 */ detail::shared_count pn;
};

} // namespace boost

#endif
