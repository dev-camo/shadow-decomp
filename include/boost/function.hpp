#ifndef BOOST_FUNCTION_HPP
#define BOOST_FUNCTION_HPP

#include "boost/utility/addressof.hpp"
#include "layout_check.h"
#include <memory>
#include <new>
#include <typeinfo>
#include <utility>

/* boost::function with one argument, in the style of boost 1.32: function1 derives from
 * function_base and keeps the manager, the functor pointer and the invoker. The game's code uses
 * a function1 holding a bind expression, so only that path is written: function objects (no plain
 * function pointers, member pointers or references), one argument, and no operator() (the unit
 * that calls the callbacks is not reconstructed yet). The bodies are visible because the game's
 * inlining depends on them (see shared_ptr.hpp). */
namespace boost {

namespace detail {
namespace function {

/* The pointer a function keeps to its functor, or to the function pointer itself. */
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

/* The result type of a function returning void: invokers return this instead. */
struct unusable {
    unusable() {}
    template <typename T> unusable(const T &) {}
};
CHECK_SIZEOF(boost::detail::function::unusable, 0x1);

template <typename T> struct function_return_type {
    typedef T type;
};

template <> struct function_return_type<void> {
    typedef unusable type;
};

enum functor_manager_operation_type {
    clone_functor_tag,
    destroy_functor_tag,
    check_functor_type_tag
};

/* Selects the manager and invoker for a class-type functor. */
struct function_obj_tag {};
CHECK_SIZEOF(boost::detail::function::function_obj_tag, 0x1);

/* Clones and destroys a functor of type Functor, and answers whether the functor has a given type
 * name. */
template <typename Functor, typename Allocator> struct functor_manager {
  private:
    typedef Functor functor_type;

    static inline any_pointer manager(any_pointer function_obj_ptr,
                                      functor_manager_operation_type op, function_obj_tag) {
        typedef typename Allocator::template rebind<functor_type>::other allocator_type;
        typedef typename allocator_type::pointer pointer_type;

        allocator_type allocator;

        if (op == clone_functor_tag) {
            functor_type *f = static_cast<functor_type *>(function_obj_ptr.obj_ptr);

            pointer_type copy = allocator.allocate(1);
            allocator.construct(copy, *f);

            functor_type *new_f = static_cast<functor_type *>(copy);
            return make_any_pointer(static_cast<void *>(new_f));
        } else {
            functor_type *f = reinterpret_cast<functor_type *>(function_obj_ptr.obj_ptr);

            pointer_type victim = static_cast<pointer_type>(f);
            allocator.destroy(victim);
            allocator.deallocate(victim, 1);

            return make_any_pointer(static_cast<void *>(0));
        }
    }

  public:
    static any_pointer manage(any_pointer functor_ptr, functor_manager_operation_type op) {
        if (op == check_functor_type_tag) {
            std::type_info *type = static_cast<std::type_info *>(functor_ptr.obj_ptr);

            return typeid(Functor) == *type ? functor_ptr
                                            : make_any_pointer(reinterpret_cast<void *>(0));
        } else {
            return manager(functor_ptr, op, function_obj_tag());
        }
    }
};

} // namespace function
} // namespace detail

/* The part of a function that does not depend on its signature. */
class function_base {
  public:
    function_base() : manager(0) {
        functor.obj_ptr = 0;
    }

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

/* Whether a functor that is itself a function is empty; any other functor is not. */
inline bool has_empty_target(const function_base *f) {
    return f->empty();
}

inline bool has_empty_target(...) {
    return false;
}

/* Calls a functor of type FunctionObj from the pointer a function keeps; the void version returns
 * the unusable placeholder because the signature's return type is void. */
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

/* boost selects the invoker with a compile-time conditional on is_void. */
template <typename FunctionObj, typename R, typename T0> struct get_function_obj_invoker1 {
    typedef function_obj_invoker1<FunctionObj, R, T0> type;
};

template <typename FunctionObj, typename T0>
struct get_function_obj_invoker1<FunctionObj, void, T0> {
    typedef void_function_obj_invoker1<FunctionObj, void, T0> type;
};

} // namespace function
} // namespace detail

template <typename R, typename T0, typename Allocator = std::allocator<function_base> >
class function1 : public function_base {
  public:
    typedef typename detail::function::function_return_type<R>::type internal_result_type;
    typedef function1 self_type;

    function1() : function_base(), invoker(0) {}

    template <typename Functor> function1(Functor f) : function_base(), invoker(0) {
        this->assign_to(f);
    }

    function1(const function1 &f) : function_base(), invoker(0) {
        this->assign_to_own(f);
    }

    ~function1() {
        clear();
    }

    function1 &operator=(const function1 &f) {
        if (&f == this) {
            return *this;
        }

        self_type(f).swap(*this);
        return *this;
    }

    void swap(function1 &other) {
        if (&other == this) {
            return;
        }

        std::swap(this->manager, other.manager);
        std::swap(this->functor, other.functor);
        std::swap(invoker, other.invoker);
    }

    void clear() {
        if (this->manager) {
            function_base::functor =
                this->manager(this->functor, detail::function::destroy_functor_tag);
        }

        this->manager = 0;
        invoker = 0;
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

            FunctionObj *new_f = static_cast<FunctionObj *>(copy);
            this->functor = detail::function::make_any_pointer(static_cast<void *>(new_f));
        }
    }

    typedef internal_result_type (*invoker_type)(detail::function::any_pointer, T0);

    /* 0x8 */ invoker_type invoker;
};

} // namespace boost

#endif
