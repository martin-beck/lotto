/**
 * @file context_payload.h
 * @brief Accessors for join-related context payloads.
 */
#ifndef LOTTO_MODULES_JOIN_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_JOIN_CONTEXT_PAYLOAD_H

#include <stdint.h>

#include <lotto/base/context.h>
#include <lotto/modules/join/events.h>
#include <lotto/runtime/context_payload.h>

typedef enum context_join_event {
    CONTEXT_JOIN_NONE = 0,
    CONTEXT_JOIN_JOIN,
    CONTEXT_JOIN_EXIT,
} context_join_event_t;

static inline context_join_event_t
context_join_event(const context_t *ctx)
{
    switch (context_event_type(ctx)) {
        case EVENT_TASK_JOIN:
            return CONTEXT_JOIN_JOIN;
        default:
            break;
    }
    switch (context_compat_category(ctx)) {
        case CAT_JOIN:
            return CONTEXT_JOIN_JOIN;
        case CAT_EXIT:
            return CONTEXT_JOIN_EXIT;
        default:
            return CONTEXT_JOIN_NONE;
    }
}

static inline uintptr_t
context_join_thread(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_event_type(ctx)) {
        case EVENT_TASK_JOIN:
            return ((join_event_t *)ctx->cp->payload)->thread;
        case EVENT_TASK_INIT:
            return context_task_init_thread(ctx);
        case EVENT_TASK_DETACH:
            return context_task_detach_thread(ctx);
        default:
            ASSERT(0);
            return 0;
    }
}

static inline bool
context_join_detached(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_INIT));
    ASSERT(context_has_capture_point(ctx));
    return context_task_init_detached(ctx);
}

static inline void **
context_join_value_ptr(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_JOIN));
    ASSERT(context_has_capture_point(ctx));
    return ((join_event_t *)ctx->cp->payload)->ptr;
}

static inline int *
context_join_ret(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_event_type(ctx)) {
        case EVENT_TASK_JOIN:
            return ((join_event_t *)ctx->cp->payload)->ret;
        case EVENT_TASK_DETACH:
            return context_task_detach_ret(ctx);
        default:
            ASSERT(0);
            return NULL;
    }
}

static inline void *
context_join_exit_value(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_FINI));
    ASSERT(context_has_capture_point(ctx));
    return context_task_fini_value(ctx);
}

#endif
