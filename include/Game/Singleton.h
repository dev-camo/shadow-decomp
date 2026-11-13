#ifndef GAME_SINGLETON_H
#define GAME_SINGLETON_H

namespace Sonicteam {
namespace System {

/* The class templates that give a class one instance, constructed on first use. Their names come
 * from RTTI: Effect::PJS::EffectManagerPJS lists
 * Sonicteam::System::Singleton<Effect::PJS::EffectManagerPJS,
 * Sonicteam::System::CreateStatic<Effect::PJS::EffectManagerPJS>> among its bases, and TCsdSystem
 * and Sonicteam::System::Diagnostics::DefaultDebugListener have type names of the same form. The
 * names of the members and the policy parameter are inferred.
 *
 * The game's code shows the same pair of functions for each such class: an accessor that stores
 * the result of a creating function in a static pointer the first time and returns the pointer,
 * and a creating function that constructs a function-local static object (registering its
 * destructor when it has one) and returns its address. Each accessor exists once, in the range of
 * the first file that calls it, and other units call it rather than expanding it. The creating
 * function is part of the accessor for USA (GUPE8P) 0x80117514 and 0x80119178, whose classes have
 * out-of-line constructors, and its own function for 0x80117578 (0x801175AC) and for the effect
 * manager (0x800A3224), whose constructors are inline. In Game/SavePoint.cpp's .sbss the guard and
 * the object of each creating function sit where the file first uses that class (0x805EEC90,
 * 0x805EECB5-0x805EECC3, with the 0x80119178 object in .bss), and the three static pointers come
 * last (0x805EECC4-0x805EECCF). Static data members of a class template are emitted after the
 * file's other data, which gives that order; a function-local static pointer in the accessor does
 * not. The creating function belongs to the policy class and not to Singleton: as a member of
 * Singleton, its name would be ambiguous in the effect manager, which derives from Singleton and
 * has virtual Create functions of its own.
 *
 * RTTI names only the instances for the effect manager, TCsdSystem and DefaultDebugListener. That
 * the three classes Game/SavePoint.cpp reaches this way (UnkManager_80117514, UnkManager_80117578
 * and Unk801A2AF4) use these templates too is inferred from their identical code. */
template <class T> class CreateStatic {
  public:
    static T *Create() {
        static T instance;

        return &instance;
    }
};

template <class T, class CreatePolicy> class Singleton {
  public:
    static T *Instance() {
        if (s_instance == 0) {
            s_instance = CreatePolicy::Create();
        }

        return s_instance;
    }

  private:
    static T *s_instance;
};

template <class T, class CreatePolicy> T *Singleton<T, CreatePolicy>::s_instance;

} // namespace System
} // namespace Sonicteam

#endif
