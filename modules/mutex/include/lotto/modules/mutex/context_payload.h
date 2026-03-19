/**
 * @file context_payload.h
 * @brief Accessors for mutex-related context payloads.
 */
#ifndef LOTTO_MODULES_MUTEX_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_MUTEX_CONTEXT_PAYLOAD_H

#include <stdint.h>

#include <lotto/base/context.h>
#include <lotto/modules/mutex/events.h>
#include <lotto/runtime/capture_point.h>

static inline uint64_t
context_mutex_addr(const context_t *ctx)
{
    if (context_has_capture_point(ctx)) {
        switch (ctx->src_type) {
            case EVENT_MUTEX_ACQUIRE:
                return (uint64_t)(uintptr_t)
                    ((mutex_acquire_event_t *)ctx->cp->payload)->addr;
            case EVENT_MUTEX_TRYACQUIRE:
                return (uint64_t)(uintptr_t)
                    ((mutex_tryacquire_event_t *)ctx->cp->payload)->addr;
            case EVENT_MUTEX_RELEASE:
                return (uint64_t)(uintptr_t)
                    ((mutex_release_event_t *)ctx->cp->payload)->addr;
            default:
                break;
        }
    }
    return (uint64_t)(uintptr_t)ctx->args[0].value.ptr;
}

static inline bool
context_mutex_try_ok(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_MUTEX_TRYACQUIRE) {
        return ((mutex_tryacquire_event_t *)ctx->cp->payload)->ret == 0;
    }
    return ctx->args[1].value.u8 == 0;
}

static inline void
context_mutex_try_set_ret(const context_t *ctx, int ret)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_MUTEX_TRYACQUIRE) {
        ((mutex_tryacquire_event_t *)ctx->cp->payload)->ret = ret;
        return;
    }
    arg_t *out = (arg_t *)&ctx->args[1];
    out->value.u8 = ret;
}

#endif
