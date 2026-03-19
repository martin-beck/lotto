/**
 * @file module_event_category.h
 * @brief Category mapping for normalized module ingress events.
 */
#ifndef LOTTO_RUNTIME_MODULE_EVENT_CATEGORY_H
#define LOTTO_RUNTIME_MODULE_EVENT_CATEGORY_H

#include <dice/events/pthread.h>
#include <dice/types.h>
#include <lotto/base/category.h>

#ifndef EVENT_MUTEX_ACQUIRE
    #define EVENT_MUTEX_ACQUIRE    142
    #define EVENT_MUTEX_TRYACQUIRE 143
    #define EVENT_MUTEX_RELEASE    144
#endif
#ifndef EVENT_EVEC_PREPARE
    #define EVENT_EVEC_PREPARE    145
    #define EVENT_EVEC_WAIT       146
    #define EVENT_EVEC_TIMED_WAIT 147
    #define EVENT_EVEC_CANCEL     148
    #define EVENT_EVEC_WAKE       149
    #define EVENT_EVEC_MOVE       150
#endif
#ifndef EVENT_RSRC_ACQUIRING
    #define EVENT_RSRC_ACQUIRING 151
    #define EVENT_RSRC_RELEASED  152
#endif
#ifndef EVENT_TASK_VELOCITY
    #define EVENT_TASK_VELOCITY 154
#endif
#ifndef EVENT_REGION_PREEMPTION
    #define EVENT_REGION_PREEMPTION 155
#endif
#ifndef EVENT_ORDER
    #define EVENT_ORDER 156
#endif
#ifndef EVENT_FORK_EXECVE
    #define EVENT_FORK_EXECVE 157
#endif
#ifndef EVENT_ROGUE
    #define EVENT_ROGUE 158
#endif
#ifndef EVENT_AWAIT
    #define EVENT_AWAIT      159
    #define EVENT_SPIN_START 160
    #define EVENT_SPIN_END   161
#endif
#ifndef EVENT_TIME_YIELD
    #define EVENT_TIME_YIELD 162
#endif
#ifndef EVENT_POLL
    #define EVENT_POLL 164
#endif
#ifndef EVENT_SCHED_YIELD
    #define EVENT_SCHED_YIELD 169
#endif
#ifndef EVENT_USER_YIELD
    #define EVENT_USER_YIELD 194
    #define EVENT_SYS_YIELD  195
#endif
#ifndef EVENT_CXA_GUARD_CALL
    #define EVENT_CXA_GUARD_CALL 175
#endif
#ifndef EVENT_TASK_JOIN
    #define EVENT_TASK_JOIN 176
#endif

static inline category_t
context_module_category(type_id type)
{
    switch (type) {
        case EVENT_MUTEX_ACQUIRE:
            return CAT_MUTEX_ACQUIRE;
        case EVENT_MUTEX_TRYACQUIRE:
            return CAT_MUTEX_TRYACQUIRE;
        case EVENT_MUTEX_RELEASE:
            return CAT_MUTEX_RELEASE;
        case EVENT_EVEC_PREPARE:
            return CAT_EVEC_PREPARE;
        case EVENT_EVEC_WAIT:
            return CAT_EVEC_WAIT;
        case EVENT_EVEC_TIMED_WAIT:
            return CAT_EVEC_TIMED_WAIT;
        case EVENT_EVEC_CANCEL:
            return CAT_EVEC_CANCEL;
        case EVENT_EVEC_WAKE:
            return CAT_EVEC_WAKE;
        case EVENT_EVEC_MOVE:
            return CAT_EVEC_MOVE;
        case EVENT_RWLOCK_RDLOCK:
        case EVENT_RWLOCK_TIMEDRDLOCK:
            return CAT_RWLOCK_RDLOCK;
        case EVENT_RWLOCK_WRLOCK:
        case EVENT_RWLOCK_TIMEDWRLOCK:
            return CAT_RWLOCK_WRLOCK;
        case EVENT_RWLOCK_UNLOCK:
            return CAT_RWLOCK_UNLOCK;
        case EVENT_RWLOCK_TRYRDLOCK:
            return CAT_RWLOCK_TRYRDLOCK;
        case EVENT_RWLOCK_TRYWRLOCK:
            return CAT_RWLOCK_TRYWRLOCK;
        case EVENT_RSRC_ACQUIRING:
            return CAT_RSRC_ACQUIRING;
        case EVENT_RSRC_RELEASED:
            return CAT_RSRC_RELEASED;
        case EVENT_SCHED_YIELD:
        case EVENT_USER_YIELD:
            return CAT_USER_YIELD;
        case EVENT_SYS_YIELD:
            return CAT_SYS_YIELD;
        case EVENT_TIME_YIELD:
            return CAT_SYS_YIELD;
        case EVENT_POLL:
            return CAT_POLL;
        case EVENT_TASK_VELOCITY:
            return CAT_TASK_VELOCITY;
        case EVENT_TASK_JOIN:
            return CAT_JOIN;
        case EVENT_REGION_PREEMPTION:
            return CAT_REGION_PREEMPTION;
        case EVENT_ORDER:
        case EVENT_FORK_EXECVE:
        case EVENT_CXA_GUARD_CALL:
            return CAT_CALL;
        default:
            return CAT_NONE;
    }
}

#endif
