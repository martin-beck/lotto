// clang-format off
#include <vsync/thread/mutex.h>
#include <vsync/thread/cond.h>
// clang-format on

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/base/context.h>
#include <lotto/modules/order/events.h>
#include <lotto/order.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
static uint64_t next_order = 1;
static vmutex_t verifier_mutex;
static vcond_t verifier_cnd;

typedef struct {
    const char *func;
    uint64_t order;
} order_event_t;

PS_ADVERTISE_TYPE(EVENT_ORDER)

void
lotto_order(uint64_t order)
{
    order_event_t ev = {
        .func  = __FUNCTION__,
        .order = order,
    };
    PS_PUBLISH(INTERCEPT_BEFORE, EVENT_ORDER, &ev, 0);

    vmutex_acquire(&verifier_mutex);

    while (order != next_order) {
        vcond_signal(&verifier_cnd);
        vcond_wait(&verifier_cnd, &verifier_mutex);
    }

    PS_PUBLISH(INTERCEPT_AFTER, EVENT_ORDER, &ev, 0);

    next_order++;

    vcond_signal(&verifier_cnd);
    vmutex_release(&verifier_mutex);
}

void
lotto_order_cond(bool cond, uint64_t order)
{
    if (!cond)
        return;

    lotto_order(order);
}

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_ORDER, {
    order_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_ORDER, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_ORDER, {
    order_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_ORDER, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_ORDER) {
        return PS_OK;
    }

    order_event_t *ev = cp->payload;
    ctx          = *origin;
    ctx.type     = EVENT_MODULE_INTERCEPT;
    ctx.src_type = cp->src_type;
    ctx.cat      = CAT_CALL;
    ctx.args[0]  = arg(uint64_t, ev->order);
    runtime_ingress_before(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_ORDER) {
        return PS_OK;
    }

    ctx          = *origin;
    ctx.type     = EVENT_MODULE_INTERCEPT;
    ctx.src_type = cp->src_type;
    ctx.cat      = CAT_CALL;
    runtime_ingress_after(&ctx);
    return PS_OK;
})
