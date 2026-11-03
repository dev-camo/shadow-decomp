/* Records the tick difference around a task's virtual update. Original names are unknown. */
#include "Game/Task.h"
#include "Game/UnkTickSource_8040ABC8.h"
#include "Game/fn_8004EDAC.h"

UnkTick fn_8004EE64(const UnkTick &second, const UnkTick &first) {
    UnkTick difference;
    difference.ticks = second.ticks - first.ticks;
    return difference;
}

void fn_8004EDAC(Task *task, float dt) {
    UnkTick first;
    UnkTick second;

    first = lbl_TickSource_805EE1E4->GetTick();
    task->Update(dt);
    second = lbl_TickSource_805EE1E4->GetTick();
    task->field_0x20 = fn_8004EE64(second, first);
}
