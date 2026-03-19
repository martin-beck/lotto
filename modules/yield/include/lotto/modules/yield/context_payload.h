/**
 * @file context_payload.h
 * @brief Accessors for yield-related context payloads.
 */
#ifndef LOTTO_MODULES_YIELD_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_YIELD_CONTEXT_PAYLOAD_H

#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/modules/yield/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>

typedef enum context_yield_event {
    CONTEXT_YIELD_NONE = 0,
    CONTEXT_YIELD_USER,
    CONTEXT_YIELD_SYS,
} context_yield_event_t;

static inline context_yield_event_t
context_yield_event(const context_t *ctx)
{
    switch (context_event_type(ctx)) {
        case EVENT_SCHED_YIELD:
        case EVENT_USER_YIELD:
            return CONTEXT_YIELD_USER;
        case EVENT_SYS_YIELD:
            return CONTEXT_YIELD_SYS;
        default:
            break;
    }
    switch (ctx->cat) {
        case CAT_USER_YIELD:
            return CONTEXT_YIELD_USER;
        case CAT_SYS_YIELD:
            return CONTEXT_YIELD_SYS;
        default:
            return CONTEXT_YIELD_NONE;
    }
}

#endif
