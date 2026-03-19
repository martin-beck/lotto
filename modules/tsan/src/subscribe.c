#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/events/pthread.h>
#include <dice/events/stacktrace.h>
#include <dice/events/thread.h>
#include <dice/interpose.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/rsrc_deadlock.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/sys/logger.h>

#define EV_PC ((uintptr_t)ev->pc)
#define PUBLISH_TSAN_INGRESS(CHAIN, SRC_TYPE, PAYLOAD, FUNC)                   \
    do {                                                                       \
        context_origin ctx =                                                   \
            *ctx_origin_pc(.self = md, .pc = EV_PC, .func = (FUNC));           \
        capture_point cp = {.src_type = (SRC_TYPE), .payload = (PAYLOAD)};     \
        PS_PUBLISH((CHAIN), EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);  \
    } while (0)

// -----------------------------------------------------------------------------
// memory accesses
//
// EVENT_MA_READ                 30       ./include/dice/events/memaccess.h
// EVENT_MA_WRITE                31       ./include/dice/events/memaccess.h
// EVENT_MA_AREAD                32       ./include/dice/events/memaccess.h
// EVENT_MA_AWRITE               33       ./include/dice/events/memaccess.h
// EVENT_MA_RMW                  34       ./include/dice/events/memaccess.h
// EVENT_MA_XCHG                 35       ./include/dice/events/memaccess.h
// EVENT_MA_CMPXCHG              36       ./include/dice/events/memaccess.h
// EVENT_MA_CMPXCHG_WEAK         37       ./include/dice/events/memaccess.h
// EVENT_MA_FENCE                38       ./include/dice/events/memaccess.h
// -----------------------------------------------------------------------------
#include <dice/events/memaccess.h>

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MA_READ, {
    struct ma_read_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_MA_READ, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MA_WRITE, {
    struct ma_write_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_MA_WRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_AREAD, {
    struct ma_aread_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_AREAD, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_AREAD, {
    struct ma_aread_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_AREAD, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_AWRITE, {
    struct ma_awrite_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_AWRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_AWRITE, {
    struct ma_awrite_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_AWRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_RMW, {
    struct ma_rmw_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_RMW, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_RMW, {
    struct ma_rmw_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_RMW, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_XCHG, {
    struct ma_xchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_XCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_XCHG, {
    struct ma_xchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_XCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_CMPXCHG, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_CMPXCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_CMPXCHG, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_CMPXCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_CMPXCHG_WEAK, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_CMPXCHG_WEAK, ev,
                         ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_CMPXCHG_WEAK, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_CMPXCHG_WEAK, ev,
                         ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_FENCE, {
    struct ma_fence_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_FENCE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_FENCE, {
    struct ma_fence_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_FENCE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_STACKTRACE_ENTER, {
    stacktrace_event_t *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_STACKTRACE_ENTER, ev,
                         "func_entry");
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_STACKTRACE_EXIT, {
    stacktrace_event_t *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_STACKTRACE_EXIT, ev, "func_exit");
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;

    switch (cp->src_type) {
        case EVENT_MA_READ:
        case EVENT_MA_WRITE:
            runtime_ingress_module_submit_memaccess(origin, cp);
            return PS_OK;
        case EVENT_STACKTRACE_ENTER:
            runtime_ingress_module_submit_event(origin, cp, EVENT_FUNC_ENTRY);
            return PS_OK;
        case EVENT_STACKTRACE_EXIT:
            runtime_ingress_module_submit_event(origin, cp, EVENT_FUNC_EXIT);
            return PS_OK;
        default:
            return PS_OK;
    }
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;

    switch (cp->src_type) {
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
        case EVENT_MA_AREAD:
        case EVENT_MA_AWRITE:
        case EVENT_MA_RMW:
        case EVENT_MA_XCHG:
        case EVENT_MA_FENCE:
            runtime_ingress_module_submit_memaccess(origin, cp);
            return PS_OK;
        default:
            return PS_OK;
    }
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;

    switch (cp->src_type) {
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
        case EVENT_MA_AREAD:
        case EVENT_MA_AWRITE:
        case EVENT_MA_RMW:
        case EVENT_MA_XCHG:
        case EVENT_MA_FENCE:
            runtime_ingress_module_submit_memaccess_after(origin, cp);
            return PS_OK;
        default:
            return PS_OK;
    }
})
