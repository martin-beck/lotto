/**
 * @file context_payload.h
 * @brief Accessors for runtime-owned context payloads.
 */
#ifndef LOTTO_RUNTIME_CONTEXT_PAYLOAD_H
#define LOTTO_RUNTIME_CONTEXT_PAYLOAD_H

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <dice/events/pthread.h>
#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/runtime/module_events.h>

typedef enum context_core_event {
    CONTEXT_CORE_NONE = 0,
    CONTEXT_CORE_TASK_CREATE,
    CONTEXT_CORE_CALL,
    CONTEXT_CORE_TASK_INIT,
    CONTEXT_CORE_TASK_FINI,
    CONTEXT_CORE_TASK_DETACH,
    CONTEXT_CORE_KEY_CREATE,
    CONTEXT_CORE_KEY_DELETE,
    CONTEXT_CORE_SET_SPECIFIC,
} context_core_event_t;

static inline context_core_event_t
context_core_event(const context_t *ctx)
{
    if (ctx == NULL) {
        return CONTEXT_CORE_NONE;
    }

    type_id type = context_event_type(ctx);
    if (type != 0) {
        switch (type) {
            case EVENT_TASK_CREATE:
                return CONTEXT_CORE_TASK_CREATE;
            case EVENT_CALL:
                return CONTEXT_CORE_CALL;
            case EVENT_TASK_INIT:
                return CONTEXT_CORE_TASK_INIT;
            case EVENT_TASK_FINI:
                return CONTEXT_CORE_TASK_FINI;
            case EVENT_TASK_DETACH:
                return CONTEXT_CORE_TASK_DETACH;
            case EVENT_KEY_CREATE:
                return CONTEXT_CORE_KEY_CREATE;
            case EVENT_KEY_DELETE:
                return CONTEXT_CORE_KEY_DELETE;
            case EVENT_SET_SPECIFIC:
                return CONTEXT_CORE_SET_SPECIFIC;
            default:
                break;
        }
    }
    return CONTEXT_CORE_NONE;
}

static inline category_t
context_core_category(type_id type)
{
    switch (type) {
        case EVENT_TASK_CREATE:
            return CAT_TASK_CREATE;
        case EVENT_CALL:
            return CAT_CALL;
        case EVENT_TASK_BLOCK:
            return CAT_TASK_BLOCK;
        case EVENT_TASK_INIT:
            return CAT_TASK_INIT;
        case EVENT_TASK_FINI:
            return CAT_TASK_FINI;
        case EVENT_TASK_DETACH:
            return CAT_DETACH;
        case EVENT_KEY_CREATE:
            return CAT_KEY_CREATE;
        case EVENT_KEY_DELETE:
            return CAT_KEY_DELETE;
        case EVENT_SET_SPECIFIC:
            return CAT_SET_SPECIFIC;
        default:
            return CAT_NONE;
    }
}

static inline category_t
context_semantic_category(type_id type)
{
    switch (type) {
        case EVENT_BEFORE_READ:
            return CAT_BEFORE_READ;
        case EVENT_BEFORE_WRITE:
            return CAT_BEFORE_WRITE;
        case EVENT_BEFORE_AREAD:
            return CAT_BEFORE_AREAD;
        case EVENT_BEFORE_AWRITE:
            return CAT_BEFORE_AWRITE;
        case EVENT_BEFORE_RMW:
            return CAT_BEFORE_RMW;
        case EVENT_BEFORE_XCHG:
            return CAT_BEFORE_XCHG;
        case EVENT_BEFORE_CMPXCHG:
            return CAT_BEFORE_CMPXCHG;
        case EVENT_BEFORE_FENCE:
            return CAT_BEFORE_FENCE;
        case EVENT_AFTER_AREAD:
            return CAT_AFTER_AREAD;
        case EVENT_AFTER_AWRITE:
            return CAT_AFTER_AWRITE;
        case EVENT_AFTER_RMW:
            return CAT_AFTER_RMW;
        case EVENT_AFTER_XCHG:
            return CAT_AFTER_XCHG;
        case EVENT_AFTER_CMPXCHG_S:
            return CAT_AFTER_CMPXCHG_S;
        case EVENT_AFTER_CMPXCHG_F:
            return CAT_AFTER_CMPXCHG_F;
        case EVENT_AFTER_FENCE:
            return CAT_AFTER_FENCE;
        case EVENT_FUNC_ENTRY:
            return CAT_FUNC_ENTRY;
        case EVENT_FUNC_EXIT:
            return CAT_FUNC_EXIT;
        case EVENT_REGION_IN:
            return CAT_REGION_IN;
        case EVENT_REGION_OUT:
            return CAT_REGION_OUT;
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

static inline category_t
context_event_category(type_id type)
{
    category_t cat = context_core_category(type);
    if (cat != CAT_NONE) {
        return cat;
    }

    cat = context_semantic_category(type);
    if (cat != CAT_NONE) {
        return cat;
    }

    return CAT_NONE;
}

static inline category_t
context_effective_category(const context_t *ctx)
{
    if (ctx == NULL) {
        return CAT_NONE;
    }

    type_id type   = context_event_type(ctx);
    category_t cat = context_event_category(type);
    if (cat != CAT_NONE) {
        return cat;
    }

    cat = context_memaccess_category(ctx);
    if (cat != CAT_NONE) {
        return cat;
    }

    return context_compat_category(ctx);
}

static inline context_t
context_finalize_category(context_t ctx)
{
    if (ctx.cat == CAT_NONE) {
        type_id type   = ctx.type != 0 ? ctx.type : ctx.src_type;
        category_t cat = context_event_category(type);
        ctx.cat = cat;
    }
    return ctx;
}

static inline context_t
context_with_types(context_t ctx, type_id type, type_id src_type)
{
    ctx.type     = type;
    ctx.src_type = src_type;
    return context_finalize_category(ctx);
}

static inline bool
context_is_task_create(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_TASK_CREATE;
}

static inline bool
context_is_call(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_CALL;
}

static inline bool
context_is_task_block(const context_t *ctx)
{
    return context_effective_category(ctx) == CAT_TASK_BLOCK;
}

static inline bool
context_is_blocking(const context_t *ctx)
{
    switch (context_core_event(ctx)) {
        case CONTEXT_CORE_TASK_CREATE:
        case CONTEXT_CORE_CALL:
            return true;
        default:
            break;
    }
    return context_is_task_block(ctx);
}

static inline bool
context_has_slack(const context_t *ctx)
{
    if (context_core_event(ctx) == CONTEXT_CORE_CALL) {
        return true;
    }
    return context_is_task_block(ctx);
}

static inline uintptr_t
context_task_init_thread(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_INIT));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->task_init->thread;
}

static inline bool
context_task_init_detached(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_INIT));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->task_init->detached;
}

static inline bool
context_is_task_init(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_TASK_INIT;
}

static inline void *
context_task_fini_value(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_FINI));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->task_fini->ptr;
}

static inline bool
context_is_task_fini(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_TASK_FINI;
}

static inline uintptr_t
context_task_detach_thread(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_DETACH));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->task_detach->thread;
}

static inline bool
context_is_task_detach(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_TASK_DETACH;
}

static inline int *
context_task_detach_ret(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_DETACH));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->task_detach->ret;
}

static inline bool
context_is_key_create(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_KEY_CREATE;
}

static inline bool
context_is_key_delete(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_KEY_DELETE;
}

static inline bool
context_is_set_specific(const context_t *ctx)
{
    return context_core_event(ctx) == CONTEXT_CORE_SET_SPECIFIC;
}

static inline pthread_key_t
context_key_value(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_event_type(ctx)) {
        case EVENT_KEY_CREATE:
            return *ctx->cp->key_create->key;
        case EVENT_KEY_DELETE:
            return ctx->cp->key_delete->key;
        case EVENT_SET_SPECIFIC:
            return ctx->cp->set_specific->key;
        default:
            ASSERT(0);
            return 0;
    }
}

static inline void (*context_key_destructor(const context_t *ctx))(void *)
{
    ASSERT(context_is_key_create(ctx));
    ASSERT(context_has_capture_point(ctx));
    return ctx->cp->key_create->destructor;
}

static inline void *
context_set_specific_value(const context_t *ctx)
{
    ASSERT(context_is_set_specific(ctx));
    ASSERT(context_has_capture_point(ctx));
    return (void *)ctx->cp->set_specific->value;
}

#endif
