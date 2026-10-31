#ifndef GAME_CSD_CSDRCOBJECT_H
#define GAME_CSD_CSDRCOBJECT_H

#include "Game/CSD/csdBase.h"

/* The library's reference-counting pointer (class name from the game's RTTI, which names
 * RCPtr<Chao::CSD::CTexList>::RCObject<Chao::CSD::CTexList> and others). This file stands in for
 * the original csdRCObject.h, the file name the game's RCPtr code passes to AllocObject: it has
 * only the members recovered code uses. Reset, Assign and the holder's constructor are defined in
 * the class because the game's copies come out right after their first user (the weak instances of
 * a member defined outside the class come out at the end of the file); the other members are
 * compiled elsewhere.
 *
 * The CBase base is inferred: the game's deleting destructor of RCPtr<unsigned char> (0x8003F224
 * in the USA release) frees the pointer through the current allocator's FreeObject, as CBase's
 * operator delete does. */
namespace Chao {
namespace CSD {

template <class T> class RCPtr : public CBase {
  public:
    RCPtr();
    RCPtr(T *object);
    ~RCPtr();
    RCPtr &operator=(const RCPtr &other);

    /* The object, or null for a pointer that holds none. */
    T *Get() const {
        return field_0x0 != NULL ? field_0x0->m_pObject : NULL;
    }
    T *operator->() const {
        return field_0x0 != NULL ? field_0x0->m_pObject : NULL;
    }

    /* Pointers are equal when they share an object holder or hold the same object. The result is
     * an int: a bool result compiles to different code in RenderState::SetTexture. */
    int operator==(const RCPtr &other) const {
        if (field_0x0 != NULL && other.field_0x0 != NULL) {
            return field_0x0->m_pObject == other.field_0x0->m_pObject;
        }
        return field_0x0 == other.field_0x0;
    }

    /* Releases the held object and takes object, returning this pointer (names inferred; the
     * game's copy at 0x8035EC38 in the USA release returns its first argument). The object is
     * released with delete (the only policy this instance uses). */
    RCPtr &Reset(T *object) {
        DeleteRelease release;

        Assign(object, release);
        return *this;
    }

    /* How a holder releases its object when the last pointer to it goes away. The switch over
     * the holder's m_eDealloctor in RCPtr<CTexList>'s code has these five cases; the names are
     * inferred from what each does. */
    enum ReleaseMode {
        RELEASE_FREE = 0,     /* Free the object's memory */
        RELEASE_DELETE = 1,   /* delete the object */
        RELEASE_NONE = 2,     /* leave the object alone */
        RELEASE_FUNCTION = 3, /* call a function with the object */
        RELEASE_RELEASER = 4  /* run a releaser object */
    };

    /* Selects RELEASE_DELETE (an empty class passed by reference; name inferred). */
    class DeleteRelease {};

    /* Holds object, released with delete; the policy object is taken by value (the RenderWare
     * texture lists pass their unit's DeleteRelease). */
    RCPtr(T *object, DeleteRelease release) : field_0x0(NULL) {
        Assign(object, release);
    }

    /* An object that releases an object itself (RELEASE_RELEASER). */
    class Releaser {
      public:
        virtual void Release();
        virtual ~Releaser();
    };

    /* The shared holder of an object (class name from the game's RTTI). The member names are
     * inferred, except m_eDealloctor, which the library's error text for an invalid value names
     * ("Not inititalized m_eDealloctor"). Holders pass this header's file name and line; the
     * original passes line 410 of its csdRCObject.h (see the unit's status in configure.py). */
    template <class U> class RCObject {
      public:
        RCObject(U *object, DeleteRelease release)
            : m_pObject(object), m_refCount(0), m_custom(), m_eDealloctor(-1) {
            m_custom.function = NULL;
            m_eDealloctor = RELEASE_DELETE;
        }

        static void *operator new(size_t size) {
            return GetMemoryAlloc()->AllocObject(size, __FILE__, __LINE__);
        }

        /* 0x0 */ U *m_pObject;
        /* 0x4 */ virtual ~RCObject();
        /* 0x8 */ int m_refCount;
        /* 0xC */ union ReleaseTarget {
            ReleaseTarget() : function(NULL) {}
            void (*function)(U *);
            Releaser *releaser;
        } m_custom;
        /* 0x10 */ int m_eDealloctor;
    };

  private:
    /* Replaces the held object by object, to be released as release selects (name inferred).
     * Called by Reset and by the RCPtr(T *, DeleteRelease) constructor. */
    void Assign(T *object, const DeleteRelease &release) {
        RCObject<T> *holder = field_0x0;

        if (holder != NULL) {
            if (holder->m_pObject == object) {
                return;
            }
            if (--holder->m_refCount == 0) {
                if (holder->m_pObject != NULL) {
                    switch (holder->m_eDealloctor) {
                    case RELEASE_FREE:
                        if (holder->m_pObject != NULL) {
                            Free(holder->m_pObject);
                            holder->m_pObject = NULL;
                        }
                        break;
                    case RELEASE_DELETE:
                        if (holder->m_pObject != NULL) {
                            delete holder->m_pObject;
                            holder->m_pObject = NULL;
                        }
                        break;
                    case RELEASE_NONE:
                        break;
                    case RELEASE_FUNCTION:
                        holder->m_custom.function(holder->m_pObject);
                        holder->m_custom.function = NULL;
                        break;
                    case RELEASE_RELEASER:
                        if (holder->m_custom.releaser != NULL) {
                            holder->m_custom.releaser->Release();
                        }
                        if (holder->m_custom.releaser != NULL) {
                            delete holder->m_custom.releaser;
                            holder->m_custom.releaser = NULL;
                        }
                        break;
                    default:
                        PrintFatalError("Not inititalized m_eDealloctor\n");
                        break;
                    }
                }
                delete holder;
            }
            field_0x0 = NULL;
        }

        if (object != NULL) {
            field_0x0 = new RCObject<T>(object, release);
            field_0x0->m_refCount++;
        }
    }

    /* 0x0 */ RCObject<T> *field_0x0;
};

} // namespace CSD
} // namespace Chao

#endif
