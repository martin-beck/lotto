#include <errno.h>

#include "dice/pubsub.h"
#include "lotto/base/arg.h"
#include <dice/chains/capture.h>
#include <dice/events/pthread.h>
#include <dice/module.h>
#include <dice/self.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/modules/join/events.h>
#include <lotto/order.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/logger.h>

#define DECL_PTHREAD_JOIN_RET(VAL)                                             \
    static int pthread_nop_##VAL##_()                                          \
    {                                                                          \
        return (VAL);                                                          \
    }
DECL_PTHREAD_JOIN_RET(0)
DECL_PTHREAD_JOIN_RET(EDEADLK)
DECL_PTHREAD_JOIN_RET(EINVAL)
DECL_PTHREAD_JOIN_RET(ESRCH)

#define CASE_PTHREAD_JOIN_RET(VAL)                                             \
    case (VAL):                                                                \
        ev->func = (void *)pthread_nop_##VAL##_;                               \
        break

typedef struct join_event {
    uintptr_t thread;
    void **ptr;
    int *ret;
} join_event_t;


PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_THREAD_JOIN, {
    struct pthread_join_event *ev = EVENT_PAYLOAD(event);
    if (self_retired(md))
        return PS_STOP_CHAIN;
    ev->ret      = EINTR;
    context_t ctx = *ctx(.self = md, .func = "pthread_join");
    join_event_t jev = {
        .thread = ev->thread,
        .ptr    = ev->ptr,
        .ret    = &ev->ret,
    };
    capture_point cp = {.src_type = EVENT_JOIN, .payload = &jev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
    switch (ev->ret) {
        CASE_PTHREAD_JOIN_RET(EDEADLK);
        CASE_PTHREAD_JOIN_RET(EINVAL);
        CASE_PTHREAD_JOIN_RET(ESRCH);
        CASE_PTHREAD_JOIN_RET(0);
        default:
            logger_fatalf("Unexpected return value\n");
    }
    return PS_STOP_CHAIN;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_THREAD_JOIN, {
    if (self_retired(md))
        return PS_STOP_CHAIN;
    //    logger_fatalf("This should not be called\n");
    return PS_STOP_CHAIN;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_JOIN) {
        return PS_OK;
    }

    join_event_t *ev = cp->payload;
    ctx          = *origin;
    ctx.type     = EVENT_MODULE_INTERCEPT;
    ctx.src_type = cp->src_type;
    ctx.cat      = CAT_JOIN;
    ctx.args[0]  = arg(uint64_t, ev->thread);
    ctx.args[1]  = arg_ptr(ev->ptr);
    ctx.args[2]  = arg_ptr(ev->ret);
    runtime_ingress(&ctx);
    return PS_OK;
})
