#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/base/context.h>
#include <lotto/modules/rogue/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/unsafe/rogue.h>

typedef struct {
    const char *func;
} rogue_event_t;

PS_ADVERTISE_TYPE(EVENT_ROGUE)

void
_lotto_region_rogue_enter()
{
    rogue_event_t ev = {.func = __FUNCTION__};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_ROGUE, &ev, 0);
}

void
_lotto_region_rogue_leave()
{
    rogue_event_t ev = {.func = __FUNCTION__};
    PS_PUBLISH(INTERCEPT_AFTER, EVENT_ROGUE, &ev, 0);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_ROGUE, {
    rogue_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_ROGUE, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_ROGUE, {
    rogue_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_ROGUE, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;

    if (cp->src_type != EVENT_ROGUE) {
        return PS_OK;
    }

    runtime_ingress(
        ctx(.self = origin->self, .pc = origin->pc, .func = origin->func,
            .cat = CAT_TASK_BLOCK));
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    capture_point *cp = (capture_point *)event;

    if (cp->src_type != EVENT_ROGUE) {
        return PS_OK;
    }

    rogue_event_t *ev = cp->payload;
    runtime_ingress_after(ev->func);
    return PS_OK;
})
