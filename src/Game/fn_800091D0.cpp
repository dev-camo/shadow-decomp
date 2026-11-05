/* The copy assignment operator (0x800091D0) and the copy constructor (0x800091EC) of the 3-float
 * vector, defined out of line. Which is which is inferred from the segment's copy constructor
 * (0x80021DE4) and assignment (0x8002CCF0), which call them for both points; the header gives
 * examples of where the original uses each. The original type and file names are unknown. */
#include "Game/fn_8040_vector_types.h"

Fn8040Vector3 &Fn8040Vector3::operator=(const Fn8040Vector3 &other) {
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}

Fn8040Vector3::Fn8040Vector3(const Fn8040Vector3 &other) : x(other.x), y(other.y), z(other.z) {}
