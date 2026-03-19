/**
 * @file events.h
 * @brief Mutex semantic ingress event identifiers.
 */
#ifndef LOTTO_MODULES_MUTEX_EVENTS_H
#define LOTTO_MODULES_MUTEX_EVENTS_H

#include <dice/types.h>

typedef struct mutex_acquire_event {
    const void *pc;
    const char *func;
    void *addr;
} mutex_acquire_event_t;

typedef struct mutex_tryacquire_event {
    const void *pc;
    const char *func;
    void *addr;
    int ret;
} mutex_tryacquire_event_t;

typedef struct mutex_release_event {
    const void *pc;
    const char *func;
    void *addr;
} mutex_release_event_t;

#define EVENT_MUTEX_ACQUIRE    142
#define EVENT_MUTEX_TRYACQUIRE 143
#define EVENT_MUTEX_RELEASE    144

#endif
