/**
 * @file context_payload.h
 * @brief Accessors for rwlock-related context payloads.
 */
#ifndef LOTTO_MODULES_RWLOCK_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_RWLOCK_CONTEXT_PAYLOAD_H

#include <stdint.h>

#include <dice/events/pthread.h>
#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>

typedef enum context_rwlock_event {
    CONTEXT_RWLOCK_NONE = 0,
    CONTEXT_RWLOCK_RDLOCK,
    CONTEXT_RWLOCK_WRLOCK,
    CONTEXT_RWLOCK_UNLOCK,
    CONTEXT_RWLOCK_TRYRDLOCK,
    CONTEXT_RWLOCK_TRYWRLOCK,
    CONTEXT_RWLOCK_TIMEDRDLOCK,
    CONTEXT_RWLOCK_TIMEDWRLOCK,
} context_rwlock_event_t;

static inline context_rwlock_event_t
context_rwlock_event(const context_t *ctx)
{
    switch (context_event_type(ctx)) {
        case EVENT_RWLOCK_RDLOCK:
            return CONTEXT_RWLOCK_RDLOCK;
        case EVENT_RWLOCK_WRLOCK:
            return CONTEXT_RWLOCK_WRLOCK;
        case EVENT_RWLOCK_UNLOCK:
            return CONTEXT_RWLOCK_UNLOCK;
        case EVENT_RWLOCK_TRYRDLOCK:
            return CONTEXT_RWLOCK_TRYRDLOCK;
        case EVENT_RWLOCK_TRYWRLOCK:
            return CONTEXT_RWLOCK_TRYWRLOCK;
        case EVENT_RWLOCK_TIMEDRDLOCK:
            return CONTEXT_RWLOCK_TIMEDRDLOCK;
        case EVENT_RWLOCK_TIMEDWRLOCK:
            return CONTEXT_RWLOCK_TIMEDWRLOCK;
        default:
            return CONTEXT_RWLOCK_NONE;
    }
}

static inline uint64_t
context_rwlock_addr(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_rwlock_event(ctx)) {
        case CONTEXT_RWLOCK_RDLOCK:
            return (uint64_t)(uintptr_t)((struct pthread_rwlock_rdlock_event *)
                                             ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_WRLOCK:
            return (uint64_t)(uintptr_t)((struct pthread_rwlock_wrlock_event *)
                                             ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_UNLOCK:
            return (uint64_t)(uintptr_t)((struct pthread_rwlock_unlock_event *)
                                             ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_TRYRDLOCK:
            return (uint64_t)(uintptr_t)((struct pthread_rwlock_tryrdlock_event
                                              *)ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_TRYWRLOCK:
            return (uint64_t)(uintptr_t)((struct pthread_rwlock_trywrlock_event
                                              *)ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_TIMEDRDLOCK:
            return (uint64_t)(uintptr_t)((struct
                                          pthread_rwlock_timedrdlock_event *)
                                             ctx->cp->payload)
                ->lock;
        case CONTEXT_RWLOCK_TIMEDWRLOCK:
            return (uint64_t)(uintptr_t)((struct
                                          pthread_rwlock_timedwrlock_event *)
                                             ctx->cp->payload)
                ->lock;
        default:
            ASSERT(0);
            return 0;
    }
}

static inline void
context_rwlock_try_set_ret(const context_t *ctx, int ret)
{
    ASSERT(context_has_capture_point(ctx));
    switch (context_rwlock_event(ctx)) {
        case CONTEXT_RWLOCK_TRYRDLOCK:
            ((struct pthread_rwlock_tryrdlock_event *)ctx->cp->payload)->ret =
                ret;
            return;
        case CONTEXT_RWLOCK_TRYWRLOCK:
            ((struct pthread_rwlock_trywrlock_event *)ctx->cp->payload)->ret =
                ret;
            return;
        default:
            ASSERT(0);
            return;
    }
}

#endif
