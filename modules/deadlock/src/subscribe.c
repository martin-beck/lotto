#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/base/category.h>
#include <lotto/modules/deadlock/events.h>
#include <lotto/rsrc_deadlock.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

typedef struct rsrc_event {
    void *addr;
} rsrc_event_t;

PS_ADVERTISE_TYPE(EVENT_RSRC_ACQUIRING)
PS_ADVERTISE_TYPE(EVENT_RSRC_RELEASED)

void
intercept_rsrc_acquiring(void *addr)
{
    rsrc_event_t ev = {.addr = addr};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_RSRC_ACQUIRING, &ev, 0);
}

void
intercept_rsrc_released(void *addr)
{
    rsrc_event_t ev = {.addr = addr};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_RSRC_RELEASED, &ev, 0);
}

void
_lotto_rsrc_acquiring(void *addr)
{
    context_t ctx  = *ctx(.self = self_md(), .func = __FUNCTION__);
    rsrc_event_t ev = {.addr = addr};
    capture_point cp = {.src_type = EVENT_RSRC_ACQUIRING, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

void
_lotto_rsrc_released(void *addr)
{
    context_t ctx  = *ctx(.self = self_md(), .func = __FUNCTION__);
    rsrc_event_t ev = {.addr = addr};
    capture_point cp = {.src_type = EVENT_RSRC_RELEASED, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_RSRC_ACQUIRING, {
    rsrc_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_rsrc_acquiring(ev->addr);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_RSRC_RELEASED, {
    rsrc_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_rsrc_released(ev->addr);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = *origin;

    switch (cp->src_type) {
        case EVENT_RSRC_ACQUIRING: {
            rsrc_event_t *ev = cp->payload;
            ctx.cat          = CAT_RSRC_ACQUIRING;
            ctx.args[0]      = arg_ptr(ev->addr);
            runtime_ingress(&ctx);
            break;
        }
        case EVENT_RSRC_RELEASED: {
            rsrc_event_t *ev = cp->payload;
            ctx.cat          = CAT_RSRC_RELEASED;
            ctx.args[0]      = arg_ptr(ev->addr);
            runtime_ingress(&ctx);
            break;
        }
        default:
            break;
    }
    return PS_OK;
})
