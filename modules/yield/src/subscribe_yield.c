#include <stdbool.h>

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/modules/yield/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

typedef struct yield_event {
    bool advisory;
} yield_event_t;

PS_ADVERTISE_TYPE(EVENT_LOTTO_YIELD)

void
intercept_yield(bool advisory)
{
    yield_event_t ev = {.advisory = advisory};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_LOTTO_YIELD, &ev, 0);
}

int
lotto_yield(bool advisory)
{
    intercept_yield(advisory);
    return 0;
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_LOTTO_YIELD, {
    yield_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = "lotto_yield");
    capture_point cp = {.src_type = EVENT_LOTTO_YIELD, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = *origin;

    if (cp->src_type != EVENT_LOTTO_YIELD) {
        return PS_OK;
    }

    yield_event_t *ev = cp->payload;
    ctx.cat           = ev->advisory ? CAT_SYS_YIELD : CAT_USER_YIELD;
    runtime_ingress(&ctx);
    return PS_OK;
})
