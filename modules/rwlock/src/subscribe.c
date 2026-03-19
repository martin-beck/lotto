#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/events/pthread.h>
#include <dice/events/thread.h>
#include <dice/interpose.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/evec.h>
#include <lotto/mutex.h>
#include <lotto/rsrc_deadlock.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/logger.h>

#define PUBLISH_RWLOCK_INGRESS(CHAIN, SRC_TYPE, PAYLOAD, PC, FUNC)             \
    do {                                                                       \
        context_origin ctx =                                                   \
            *ctx_origin_pc(.self = md, .pc = (uintptr_t)(PC), .func = (FUNC)); \
        capture_point cp = {.src_type = (SRC_TYPE), .payload = (PAYLOAD)};     \
        PS_PUBLISH((CHAIN), EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);  \
    } while (0)

static int
const_zero(pthread_rwlock_t *l)
{
    return 0;
}
static int
const_zero_timed(pthread_rwlock_t *l, const struct timespec *t)
{
    return 0;
}

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_RDLOCK, {
    struct pthread_rwlock_rdlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_RWLOCK_RDLOCK, ev,
                           ev->pc, "pthread_rwlock_rdlock");
    ev->func = const_zero;
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_WRLOCK, {
    struct pthread_rwlock_wrlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_RWLOCK_WRLOCK, ev,
                           ev->pc, "pthread_rwlock_wrlock");
    ev->func = const_zero;
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_UNLOCK, {
    struct pthread_rwlock_unlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_RWLOCK_UNLOCK, ev,
                           ev->pc, "pthread_rwlock_unlock");
    ev->func = const_zero;
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_TRYRDLOCK, {
    struct pthread_rwlock_tryrdlock_event *ev = EVENT_PAYLOAD(event);
    ev->func                                  = const_zero;
    return PS_OK;
})
PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_RWLOCK_TRYRDLOCK, {
    struct pthread_rwlock_tryrdlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_AFTER, EVENT_RWLOCK_TRYRDLOCK, ev,
                           ev->pc, "pthread_rwlock_tryrdlock");
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_TRYWRLOCK, {
    struct pthread_rwlock_trywrlock_event *ev = EVENT_PAYLOAD(event);
    ev->func                                  = const_zero;
    return PS_OK;
})
PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_RWLOCK_TRYWRLOCK, {
    struct pthread_rwlock_trywrlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_AFTER, EVENT_RWLOCK_TRYWRLOCK, ev,
                           ev->pc, "pthread_rwlock_trywrlock");
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_TIMEDRDLOCK, {
    struct pthread_rwlock_timedrdlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_RWLOCK_TIMEDRDLOCK, ev,
                           ev->pc, "pthread_rwlock_timedrdlock");
    ev->func = const_zero_timed;
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_RWLOCK_TIMEDWRLOCK, {
    struct pthread_rwlock_timedwrlock_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_RWLOCK_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_RWLOCK_TIMEDWRLOCK, ev,
                           ev->pc, "pthread_rwlock_timedwrlock");
    ev->func = const_zero_timed;
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;
    switch (cp->src_type) {
        case EVENT_RWLOCK_RDLOCK: {
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        }
        case EVENT_RWLOCK_WRLOCK: {
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        }
        case EVENT_RWLOCK_UNLOCK: {
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        }
        case EVENT_RWLOCK_TIMEDRDLOCK: {
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        }
        case EVENT_RWLOCK_TIMEDWRLOCK: {
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        }
        default:
            return PS_OK;
    }
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;
    switch (cp->src_type) {
        case EVENT_RWLOCK_TRYRDLOCK:
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        case EVENT_RWLOCK_TRYWRLOCK:
            runtime_ingress_module_submit_auto(origin, cp);
            return PS_OK;
        default:
            return PS_OK;
    }
})
