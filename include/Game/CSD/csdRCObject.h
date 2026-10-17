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
    RCPtr(T *object);
    ~RCPtr();

  private:
    template <class U> class RCObject;

    /* 0x0 */ RCObject<T> *field_0x0;
};

} // namespace CSD
} // namespace Chao

#endif
