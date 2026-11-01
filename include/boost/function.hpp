#ifndef BOOST_FUNCTION_HPP
#define BOOST_FUNCTION_HPP

// Boost.Function library
//
// function/function_base.hpp
//
//  Copyright Douglas Gregor 2001-2004. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// function/function_template.hpp
//
//  Copyright Douglas Gregor 2001-2003. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// function/function1.hpp
//
//  Copyright Douglas Gregor 2002-2003. Use, modification and
//  distribution is subject to the Boost Software License, Version
//  1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// For more information, see http://www.boost.org
//
// Adapted from Boost 1.32, with the include guard following this project's naming:
// boost/function/function_base.hpp and the function1 instance of
// boost/function/function_template.hpp that boost/function/function1.hpp generates (the
// one-argument boost::function, which the game uses directly as boost::function1). The preprocessor repetition for other argument counts is not
// reproduced, and the pieces that depend on type traits the reconstructed units never need are
// left out: plain function pointers, member pointers, boost::ref and stateless function objects
// (the reconstructed units store only boost::bind expressions), target(), the comparison against
// other functors. The bad_function_call class retains Boost's inline constructor and implicit
// destructor. The typedefs and constants no reconstructed unit refers to are also left out:
// args, arity, sig and arg1_type. The configuration macros are resolved for MWCC
// (BOOST_NO_SFINAE), and BOOST_ASSERT is resolved for an NDEBUG build, so the
// checks in the int constructor and assignment are left out. The bodies stay in the classes as in
// Boost: the game's inlining depends on them (see cflags_game_cpp in configure.py).

#include "boost/utility/addressof.hpp"
#include "boost/throw_exception.hpp"
#include <stdexcept>
#include <memory>
#include <new>
#include <typeinfo>
#include <utility>
#include "layout_check.h"

namespace boost {

class bad_function_call : public std::runtime_error {
  public:
    bad_function_call() : std::runtime_error("call to empty boost::function") {}
};
CHECK_SIZEOF(boost::bad_function_call, 0xC);

namespace detail {
namespace function {

/**
 * A union of a function pointer and a void pointer. This is necessary
 * because 5.2.10/6 allows reinterpret_cast<> to safely cast between
 * function pointer types and 5.2.9/10 allows static_cast<> to safely
 * cast between a void pointer and an object pointer. But it is not legal
 * to cast between a function pointer and a void* (in either direction),
 * so function requires a union of the two. */
union any_pointer {
    void *obj_ptr;
    const void *const_obj_ptr;
    void (*func_ptr)();
    char data[1];
};
CHECK_SIZEOF(boost::detail::function::any_pointer, 0x4);

inline any_pointer make_any_pointer(void *o) {
    any_pointer p;
    p.obj_ptr = o;
    return p;
}

/**
 * The unusable class is a placeholder for unused function arguments
 * It is also completely unusable except that it constructable from
 * anything. This helps compilers without partial specialization to
 * handle Boost.Function objects returning void.
 */
struct unusable {
    unusable() {}
    template <typename T> unusable(const T &) {}
};
CHECK_SIZEOF(boost::detail::function::unusable, 0x1);

/* Determine the return type. This supports compilers that do not support
 * void returns or partial specialization by silently changing the return
 * type to "unusable".
 */
template <typename T> struct function_return_type {
    typedef T type;
};

template <> struct function_return_type<void> {
    typedef unusable type;
};

// The operation type to perform on the given functor/function pointer
enum functor_manager_operation_type {
    clone_functor_tag,
    destroy_functor_tag,
    check_functor_type_tag
};

// Tags used to decide between different types of functions
struct function_obj_tag {};
CHECK_SIZEOF(boost::detail::function::function_obj_tag, 0x1);

/**
 * The functor_manager class contains a static function "manage" which
 * can clone or destroy the given function/function object pointer.
 */
template <typename Functor, typename Allocator> struct functor_manager {
  private:
    typedef Functor functor_type;

    // For function object pointers, we clone the pointer to each
    // function has its own version.
    static inline any_pointer manager(any_pointer function_obj_ptr,
                                      functor_manager_operation_type op, function_obj_tag) {
        typedef typename Allocator::template rebind<functor_type>::other allocator_type;
        typedef typename allocator_type::pointer pointer_type;

        allocator_type allocator;

        if (op == clone_functor_tag) {
            functor_type *f = static_cast<functor_type *>(function_obj_ptr.obj_ptr);

            // Clone the functor
            pointer_type copy = allocator.allocate(1);
            allocator.construct(copy, *f);

            // Get back to the original pointer type
            functor_type *new_f = static_cast<functor_type *>(copy);
            return make_any_pointer(static_cast<void *>(new_f));
        } else {
            /* Cast from the void pointer to the functor pointer type */
            functor_type *f = reinterpret_cast<functor_type *>(function_obj_ptr.obj_ptr);

            /* Cast from the functor pointer type to the allocator's pointer
               type */
            pointer_type victim = static_cast<pointer_type>(f);

            // Destroy and deallocate the functor
            allocator.destroy(victim);
            allocator.deallocate(victim, 1);

            return make_any_pointer(static_cast<void *>(0));
        }
    }

  public:
    /* Dispatch to an appropriate manager based on whether we have a
       function pointer or a function object pointer. */
    static any_pointer manage(any_pointer functor_ptr, functor_manager_operation_type op) {
        if (op == check_functor_type_tag) {
            std::type_info *type = static_cast<std::type_info *>(functor_ptr.obj_ptr);
            return (typeid(Functor) == *type ? functor_ptr
                                             : make_any_pointer(reinterpret_cast<void *>(0)));
        } else {
            return manager(functor_ptr, op, function_obj_tag());
        }
    }
};

} // end namespace function
} // end namespace detail

/**
 * The function_base class contains the basic elements needed for the
 * function1, function2, function3, etc. classes. It is common to all
 * functions (and as such can be used to tell if we have one of the
 * functionN objects).
 */
class function_base {
  public:
    function_base() : manager(0) {
        functor.obj_ptr = 0;
    }

    // Is this function empty?
    bool empty() const {
        return !manager;
    }

    /* 0x0 */ detail::function::any_pointer (*manager)(
        detail::function::any_pointer, detail::function::functor_manager_operation_type);
    /* 0x4 */ detail::function::any_pointer functor;
};
CHECK_SIZEOF(boost::function_base, 0x8);

namespace detail {
namespace function {

/**
 * Determine if the given function object is empty: a function object that is
 * itself a boost::function is empty when it has no target; any other
 * function object never is.
 */
inline bool has_empty_target(const function_base *f) {
    return f->empty();
}

inline bool has_empty_target(...) {
    return false;
}

template <typename FunctionObj, typename R, typename T0> struct function_obj_invoker1 {
    static R invoke(any_pointer function_obj_ptr, T0 a0) {
        FunctionObj *f = (FunctionObj *)(function_obj_ptr.obj_ptr);
        return (*f)(a0);
    }
};

template <typename FunctionObj, typename R, typename T0> struct void_function_obj_invoker1 {
    static unusable invoke(any_pointer function_obj_ptr, T0 a0) {
        FunctionObj *f = (FunctionObj *)(function_obj_ptr.obj_ptr);
        (*f)(a0);
        return unusable();
    }
};

// boost selects the invoker with ct_if<is_void<R>::value, ...>, resolved here with a
// specialization for void.
template <typename FunctionObj, typename R, typename T0> struct get_function_obj_invoker1 {
    typedef function_obj_invoker1<FunctionObj, R, T0> type;
};

template <typename FunctionObj, typename T0>
struct get_function_obj_invoker1<FunctionObj, void, T0> {
    typedef void_function_obj_invoker1<FunctionObj, void, T0> type;
};

} // end namespace function
} // end namespace detail

template <typename R, typename T0, typename Allocator = std::allocator<function_base> >
class function1 : public function_base {
  public:
    typedef typename detail::function::function_return_type<R>::type internal_result_type;

    typedef T0 argument_type;

    typedef R result_type;
    typedef Allocator allocator_type;
    typedef function1 self_type;

    function1() : function_base(), invoker(0) {}

    // MSVC chokes if the following two constructors are collapsed into
    // one with a default parameter.
    template <typename Functor> function1(Functor f) : function_base(), invoker(0) {
        this->assign_to(f);
    }

    function1(int zero) : function_base(), invoker(0) {}

    function1(const function1 &f) : function_base(), invoker(0) {
        this->assign_to_own(f);
    }

    ~function1() {
        clear();
    }

    // The distinction between when to use function1 and
    // when to use self_type is obnoxious. MSVC cannot handle self_type as
    // the return type of these assignment operators, but Borland C++ cannot
    // handle function1 as the type of the temporary to
    // construct.
    template <typename Functor> function1 &operator=(Functor f) {
        self_type(f).swap(*this);
        return *this;
    }

    function1 &operator=(int zero) {
        this->clear();
        return *this;
    }

    // Assignment from another function1
    function1 &operator=(const function1 &f) {
        if (&f == this)
            return *this;

        self_type(f).swap(*this);
        return *this;
    }

    result_type operator()(T0 a0) const {
        if (this->empty()) {
            boost::throw_exception(bad_function_call());
        }
        internal_result_type result = invoker(this->functor, a0);
        return static_cast<result_type>(result);
    }

    void swap(function1 &other) {
        if (&other == this)
            return;

        std::swap(this->manager, other.manager);
        std::swap(this->functor, other.functor);
        std::swap(invoker, other.invoker);
    }

    // Clear out a target, if there is one
    void clear() {
        if (this->manager) {
            function_base::functor =
                this->manager(this->functor, detail::function::destroy_functor_tag);
        }

        this->manager = 0;
        invoker = 0;
    }

  private:
    struct dummy {
        void nonnull() {}
    };

    typedef void (dummy::*safe_bool)();

  public:
    operator safe_bool() const {
        return (this->empty()) ? 0 : &dummy::nonnull;
    }

    bool operator!() const {
        return this->empty();
    }

  private:
    void assign_to_own(const function1 &f) {
        if (!f.empty()) {
            invoker = f.invoker;
            this->manager = f.manager;
            this->functor = f.manager(f.functor, detail::function::clone_functor_tag);
        }
    }

    template <typename Functor> void assign_to(Functor f) {
        this->assign_to(f, detail::function::function_obj_tag());
    }

    template <typename FunctionObj>
    void assign_to(FunctionObj f, detail::function::function_obj_tag) {
        if (!detail::function::has_empty_target(addressof(f))) {
            typedef typename detail::function::get_function_obj_invoker1<FunctionObj, R, T0>::type
                actual_invoker_type;

            invoker = &actual_invoker_type::invoke;
            this->manager = &detail::function::functor_manager<FunctionObj, Allocator>::manage;

            typedef typename Allocator::template rebind<FunctionObj>::other rebound_allocator_type;
            typedef typename rebound_allocator_type::pointer pointer_type;

            rebound_allocator_type allocator;
            pointer_type copy = allocator.allocate(1);
            allocator.construct(copy, f);

            // Get back to the original pointer type
            FunctionObj *new_f = static_cast<FunctionObj *>(copy);

            this->functor = detail::function::make_any_pointer(static_cast<void *>(new_f));
        }
    }

    typedef internal_result_type (*invoker_type)(detail::function::any_pointer, T0);

    /* 0x8 */ invoker_type invoker;
};

template <typename R, typename T0, typename Allocator>
inline void swap(function1<R, T0, Allocator> &f1, function1<R, T0, Allocator> &f2) {
    f1.swap(f2);
}

// Poison comparisons between boost::function objects of the same type.
template <typename R, typename T0, typename Allocator>
void operator==(const function1<R, T0, Allocator> &, const function1<R, T0, Allocator> &);

template <typename R, typename T0, typename Allocator>
void operator!=(const function1<R, T0, Allocator> &, const function1<R, T0, Allocator> &);

} // end namespace boost

#endif
