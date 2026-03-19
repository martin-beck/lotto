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
        case EVENT_JOIN:
            return CONTEXT_JOIN_JOIN;
        default:
            break;
    }
    switch (ctx->cat) {
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
    if (context_has_capture_point(ctx)) {
        switch (context_event_type(ctx)) {
            case EVENT_JOIN:
                return ((join_event_t *)ctx->cp->payload)->thread;
            case EVENT_TASK_INIT:
                return context_task_init_thread(ctx);
            case EVENT_TASK_DETACH:
                return context_task_detach_thread(ctx);
            default:
                break;
        }
    }
    return (uintptr_t)ctx->args[0].value.u64;
}

static inline bool
context_join_detached(const context_t *ctx)
{
    if (context_has_event_type(ctx, EVENT_TASK_INIT) &&
        context_has_capture_point(ctx)) {
        return context_task_init_detached(ctx);
    }
    return ctx->args[1].value.u8;
}

static inline void **
context_join_value_ptr(const context_t *ctx)
{
    if (context_has_event_type(ctx, EVENT_JOIN) && context_has_capture_point(ctx)) {
        return ((join_event_t *)ctx->cp->payload)->ptr;
    }
    return (void **)ctx->args[1].value.ptr;
}

static inline int *
context_join_ret(const context_t *ctx)
{
    if (context_has_capture_point(ctx)) {
        switch (context_event_type(ctx)) {
            case EVENT_JOIN:
                return ((join_event_t *)ctx->cp->payload)->ret;
            case EVENT_TASK_DETACH:
                return context_task_detach_ret(ctx);
            default:
                break;
        }
    }
    if (context_is_task_detach(ctx)) {
        return (int *)ctx->args[1].value.ptr;
    }
    return (int *)ctx->args[2].value.ptr;
}

static inline void *
context_join_exit_value(const context_t *ctx)
{
    if (context_has_event_type(ctx, EVENT_TASK_FINI) &&
        context_has_capture_point(ctx)) {
        return context_task_fini_value(ctx);
    }
    return (void *)ctx->args[0].value.ptr;
}

#endif
