/* Mission::TimerCountMission (class name from RTTI): derives from CountMission and TimerMission.
 * Each override except fn_8016AF38, which returns 6, passes the call on to the base that implements
 * it; the overrides of fn_8016AF40, fn_8016AF44 and fn_8016AF48 call both bases, the CountMission
 * part first. File name inferred from the class. Definitions are in reverse address order
 * (deferred inlining emits functions last-defined first). */
#include "Game/TimerCountMission.h"

namespace Mission {

TimerCountMission::TimerCountMission(int id, int index, int count, float time)
    : Mission(id, index), CountMission(id, index, count, 0, 0, 0), TimerMission(id, index, time) {}

TimerCountMission::~TimerCountMission() {}

void TimerCountMission::fn_8016AF40() {
    CountMission::fn_8016AF40();
    TimerMission::fn_8016AF40();
}

void TimerCountMission::fn_8016AF44() {
    CountMission::fn_8016AF44();
    TimerMission::fn_8016AF44();
}

void TimerCountMission::fn_8016AF48() {
    CountMission::fn_8016AF48();
    TimerMission::fn_8016AF48();
}

int TimerCountMission::fn_8016AF38() {
    return 6;
}

void TimerCountMission::fn_8016AA98() {
    TimerMission::fn_8016AA98();
}

int TimerCountMission::fn_8016B240(int which) {
    return CountMission::fn_8016B240(which);
}

int TimerCountMission::fn_8016B238() {
    return CountMission::fn_8016B238();
}

int TimerCountMission::fn_8016B230() {
    return CountMission::fn_8016B230();
}

void TimerCountMission::fn_8016B224(int value, int which) {
    CountMission::fn_8016B224(value, which);
}

void TimerCountMission::fn_8016B220(int value, int which) {
    CountMission::fn_8016B220(value, which);
}

} // namespace Mission
