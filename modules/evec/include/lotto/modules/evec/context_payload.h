/**
 * @file context_payload.h
 * @brief Accessors for evec-related context payloads.
 */
#ifndef LOTTO_MODULES_EVEC_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_EVEC_CONTEXT_PAYLOAD_H

#include <stdint.h>
#include <time.h>

#include <lotto/base/context.h>
#include <lotto/modules/evec/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>

typedef enum context_evec_event {
    CONTEXT_EVEC_NONE = 0,
    CONTEXT_EVEC_PREPARE,
    CONTEXT_EVEC_WAIT,
    CONTEXT_EVEC_TIMED_WAIT,
    CONTEXT_EVEC_CANCEL,
    CONTEXT_EVEC_WAKE,
    CONTEXT_EVEC_MOVE,
} context_evec_event_t;

static inline context_evec_event_t
context_evec_event(const context_t *ctx)
{
    switch (context_event_type(ctx)) {
        case EVENT_EVEC_PREPARE:
            return CONTEXT_EVEC_PREPARE;
        case EVENT_EVEC_WAIT:
            return CONTEXT_EVEC_WAIT;
        case EVENT_EVEC_TIMED_WAIT:
            return CONTEXT_EVEC_TIMED_WAIT;
        case EVENT_EVEC_CANCEL:
            return CONTEXT_EVEC_CANCEL;
        case EVENT_EVEC_WAKE:
            return CONTEXT_EVEC_WAKE;
        case EVENT_EVEC_MOVE:
            return CONTEXT_EVEC_MOVE;
        default:
            break;
    }
    switch (context_compat_category(ctx)) {
        case CAT_EVEC_PREPARE:
            return CONTEXT_EVEC_PREPARE;
        case CAT_EVEC_WAIT:
            return CONTEXT_EVEC_WAIT;
        case CAT_EVEC_TIMED_WAIT:
            return CONTEXT_EVEC_TIMED_WAIT;
        case CAT_EVEC_CANCEL:
            return CONTEXT_EVEC_CANCEL;
        case CAT_EVEC_WAKE:
            return CONTEXT_EVEC_WAKE;
        case CAT_EVEC_MOVE:
            return CONTEXT_EVEC_MOVE;
        default:
            return CONTEXT_EVEC_NONE;
    }
}

static inline uint64_t
context_evec_id(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_evec_event(ctx)) {
        case CONTEXT_EVEC_PREPARE:
            return (uint64_t)(uintptr_t)((evec_prepare_event_t *)
                                             ctx->cp->payload)
                ->addr;
        case CONTEXT_EVEC_WAIT:
            return (uint64_t)(uintptr_t)((evec_wait_event_t *)ctx->cp->payload)
                ->addr;
        case CONTEXT_EVEC_TIMED_WAIT:
            return (uint64_t)(uintptr_t)((evec_timed_wait_event_t *)
                                             ctx->cp->payload)
                ->addr;
        case CONTEXT_EVEC_CANCEL:
            return (uint64_t)(uintptr_t)((evec_cancel_event_t *)
                                             ctx->cp->payload)
                ->addr;
        case CONTEXT_EVEC_WAKE:
            return (uint64_t)(uintptr_t)((evec_wake_event_t *)ctx->cp->payload)
                ->addr;
        case CONTEXT_EVEC_MOVE:
            return (uint64_t)(uintptr_t)((evec_move_event_t *)ctx->cp->payload)
                ->src;
        default:
            ASSERT(0);
            return 0;
    }
}

static inline const struct timespec *
context_evec_abstime(const context_t *ctx)
{
    ASSERT(context_evec_event(ctx) == CONTEXT_EVEC_TIMED_WAIT);
    ASSERT(context_has_capture_point(ctx));
    return ((evec_timed_wait_event_t *)ctx->cp->payload)->abstime;
}

static inline enum lotto_timed_wait_status *
context_evec_timed_wait_ret(const context_t *ctx)
{
    ASSERT(context_evec_event(ctx) == CONTEXT_EVEC_TIMED_WAIT);
    ASSERT(context_has_capture_point(ctx));
    return &((evec_timed_wait_event_t *)ctx->cp->payload)->ret;
}

static inline uint32_t
context_evec_wake_count(const context_t *ctx)
{
    ASSERT(context_evec_event(ctx) == CONTEXT_EVEC_WAKE);
    ASSERT(context_has_capture_point(ctx));
    return ((evec_wake_event_t *)ctx->cp->payload)->cnt;
}

static inline uint64_t
context_evec_move_dst(const context_t *ctx)
{
    ASSERT(context_evec_event(ctx) == CONTEXT_EVEC_MOVE);
    ASSERT(context_has_capture_point(ctx));
    return (uint64_t)(uintptr_t)((evec_move_event_t *)ctx->cp->payload)->dst;
}

#endif
