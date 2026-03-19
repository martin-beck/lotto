/**
 * @file events.h
 * @brief Join semantic ingress event identifiers.
 */
#ifndef LOTTO_MODULES_JOIN_EVENTS_H
#define LOTTO_MODULES_JOIN_EVENTS_H

#include <stdint.h>

#define EVENT_TASK_JOIN 176

typedef struct join_event {
    uintptr_t thread;
    void **ptr;
    int *ret;
} join_event_t;

#endif
