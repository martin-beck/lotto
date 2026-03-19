#include <dice/chains/capture.h>
#include <dice/self.h>
#include <dice/events/cxa.h>
#include <dice/events/thread.h>
#include <dice/interpose.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/modules/cxa/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_CXA_GUARD_ACQUIRE, {
    struct __cxa_guard_acquire_event *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx_pc(.self = md, .pc = (uintptr_t)ev->pc,
                               .func = "__cxa_guard_acquire");
    capture_point cp = {.src_type = EVENT_CXA_GUARD_CALL, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_CXA_GUARD_ACQUIRE, {
    context_t ctx    = *ctx(.self = md, .func = "__cxa_guard_acquire");
    capture_point cp = {.src_type = EVENT_CXA_GUARD_CALL, .payload = NULL};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_CXA_GUARD_CALL) {
        return PS_OK;
    }

    struct __cxa_guard_acquire_event *ev = cp->payload;
    ctx         = *origin;
    ctx.cat     = CAT_CALL;
    ctx.args[0] = arg_ptr(ev->addr);
    (void)runtime_ingress_before(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;

    if (cp->src_type != EVENT_CXA_GUARD_CALL) {
        return PS_OK;
    }

    runtime_ingress_after(origin->func);
    return PS_OK;
})
