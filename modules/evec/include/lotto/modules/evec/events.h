/**
 * @file events.h
 * @brief Evec semantic ingress event identifiers.
 */
#ifndef LOTTO_MODULES_EVEC_EVENTS_H
#define LOTTO_MODULES_EVEC_EVENTS_H

#include <stdint.h>
#include <time.h>

#include <lotto/evec.h>

#define EVENT_EVEC_PREPARE    145
#define EVENT_EVEC_WAIT       146
#define EVENT_EVEC_TIMED_WAIT 147
#define EVENT_EVEC_CANCEL     148
#define EVENT_EVEC_WAKE       149
#define EVENT_EVEC_MOVE       150

typedef struct evec_prepare_event {
    void *addr;
} evec_prepare_event_t;

typedef struct evec_wait_event {
    void *addr;
} evec_wait_event_t;

typedef struct evec_timed_wait_event {
    void *addr;
    const struct timespec *abstime;
    enum lotto_timed_wait_status ret;
} evec_timed_wait_event_t;

typedef struct evec_cancel_event {
    void *addr;
} evec_cancel_event_t;

typedef struct evec_wake_event {
    void *addr;
    uint32_t cnt;
} evec_wake_event_t;

typedef struct evec_move_event {
    void *src;
    void *dst;
} evec_move_event_t;

#endif
