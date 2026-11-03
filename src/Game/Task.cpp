/* Task is the class name from RTTI; the original source filename is unknown. */
#include <Game/Task.h>

extern u32 lbl_Task_805EE1D8;
const char *lbl_805E3978 = "Task";

Task::Task(Task *parent) {
    m_flags = 0;
#if !defined(VERSION_GUPE8P)
    fn_80050C40();
#endif
    field_0x20 = UnkTick(0ULL);
    m_parent = parent;
    m_child = 0;
    m_name = lbl_805E3978;
    if (parent == 0) {
        m_prev = this;
        m_next = 0;
    } else {
        Task *first_child = parent->m_child;
        if (first_child != 0) {
            m_prev = first_child->m_prev;
            m_next = 0;
            first_child->m_prev->m_next = this;
            first_child->m_prev = this;
        } else {
            m_prev = this;
            parent->m_child = this;
            m_next = 0;
        }
    }
    ++lbl_Task_805EE1D8;
#if !defined(VERSION_GUPE8P)
    fn_80050BF8();
#endif
}

Task::~Task() {
#if !defined(VERSION_GUPE8P)
    fn_80050C40();
#endif
    m_flags |= 0x20;
    if (m_parent != 0) {
        if (m_prev == this) {
            m_parent->m_child = 0;
        } else if (m_parent->m_child == this) {
            m_parent->m_child = m_next;
            m_prev->m_next = 0;
            if (m_next != 0) {
                m_next->m_prev = m_prev;
            }
        } else {
            m_prev->m_next = m_next;
            if (m_next != 0) {
                m_next->m_prev = m_prev;
            } else {
                m_parent->m_child->m_prev = m_prev;
            }
        }
    }
    --lbl_Task_805EE1D8;
#if !defined(VERSION_GUPE8P)
    fn_80050BF8();
#endif
}
