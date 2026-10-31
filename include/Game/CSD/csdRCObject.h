#ifndef GAME_CSD_CSDRCOBJECT_H
#define GAME_CSD_CSDRCOBJECT_H

#include "Game/CSD/csdBase.h"

/* The library's reference-counting pointer (class name from the game's RTTI, which names
 * RCPtr<Chao::CSD::CTexList>::RCObject<Chao::CSD::CTexList> and others). This file stands in for
 * the original csdRCObject.h, the file name the game's RCPtr code passes to AllocObject: it declares
 * only the members recovered code uses; the game's copies of the template members are compiled
 * elsewhere.
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
     * game's copy at 0x8035EC38 in the USA release returns its first argument). */
    RCPtr &Reset(T *object);

  private:
    /* The shared holder of an object (class name from the game's RTTI, member name inferred; the
     * members after the object are not recovered yet). */
    template <class U> class RCObject {
      public:
        /* 0x0 */ U *m_pObject;
        /* 0x4 */ virtual ~RCObject();
    };

    /* 0x0 */ RCObject<T> *field_0x0;
};

} // namespace CSD
} // namespace Chao

#endif
