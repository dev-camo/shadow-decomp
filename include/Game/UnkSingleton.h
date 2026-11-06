#ifndef GAME_UNKSINGLETON_H
#define GAME_UNKSINGLETON_H

/* A class template that gives a class one instance, constructed on first use (placeholder name;
 * the template form and the names of the template and its members are inferred). The game's code
 * shows the same pair of functions for three classes: an accessor that stores the result of a
 * creating function in a static pointer the first time and returns the pointer, and a creating
 * function that constructs a function-local static object (registering its destructor when it has
 * one) and returns its address. Each accessor exists once, in this file's range, and other units
 * call it rather than expanding it. The creating function is part of the accessor for USA (GUPE8P)
 * 0x80117514 and 0x80119178, whose classes have out-of-line constructors, and its own function
 * (0x801175AC) for 0x80117578, whose constructor is inline. In Game/SavePoint.cpp's .sbss the
 * guard and the object of each creating function sit where the file first uses that class
 * (0x805EEC90, 0x805EECB5-0x805EECC3, with the 0x80119178 object in .bss), and the three static
 * pointers come last (0x805EECC4-0x805EECCF). Static data members of a class template are emitted
 * after the file's other data, which gives that order; a function-local static pointer in the
 * accessor does not. */
template <class T> class UnkSingleton {
  public:
    static T *Instance() {
        if (s_instance == 0) {
            s_instance = Create();
        }

        return s_instance;
    }

  private:
    static T *Create() {
        static T instance;

        return &instance;
    }

    static T *s_instance;
};

template <class T> T *UnkSingleton<T>::s_instance;

#endif
