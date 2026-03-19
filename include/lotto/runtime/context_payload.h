/**
 * @file context_payload.h
 * @brief Accessors for runtime-owned context payloads.
 */
#ifndef LOTTO_RUNTIME_CONTEXT_PAYLOAD_H
#define LOTTO_RUNTIME_CONTEXT_PAYLOAD_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress_events.h>

static inline uintptr_t
context_task_init_thread(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_INIT) {
        return ctx->cp->task_init->thread;
    }
    return (uintptr_t)ctx->args[0].value.u64;
}

static inline bool
context_task_init_detached(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_INIT) {
        return ctx->cp->task_init->detached;
    }
    return ctx->args[1].value.u8;
}

static inline void *
context_task_fini_value(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_FINI) {
        return ctx->cp->task_fini->ptr;
    }
    return (void *)ctx->args[0].value.ptr;
}

static inline uintptr_t
context_task_detach_thread(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_DETACH) {
        return ctx->cp->task_detach->thread;
    }
    return (uintptr_t)ctx->args[0].value.u64;
}

static inline int *
context_task_detach_ret(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_DETACH) {
        return ctx->cp->task_detach->ret;
    }
    return (int *)ctx->args[1].value.ptr;
}

static inline pthread_key_t
context_key_value(const context_t *ctx)
{
    if (context_has_capture_point(ctx)) {
        switch (ctx->type) {
            case EVENT_KEY_CREATE:
                return *ctx->cp->key_create->key;
            case EVENT_KEY_DELETE:
                return ctx->cp->key_delete->key;
            case EVENT_SET_SPECIFIC:
                return ctx->cp->set_specific->key;
            default:
                break;
        }
    }
    return *(pthread_key_t *)ctx->args[0].value.ptr;
}

static inline void (*context_key_destructor(const context_t *ctx))(void *)
{
    if (context_has_capture_point(ctx) && ctx->type == EVENT_KEY_CREATE) {
        return ctx->cp->key_create->destructor;
    }
    return (void (*)(void *))ctx->args[1].value.ptr;
}

static inline void *
context_set_specific_value(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->type == EVENT_SET_SPECIFIC) {
        return (void *)ctx->cp->set_specific->value;
    }
    return (void *)ctx->args[1].value.ptr;
}

#endif
