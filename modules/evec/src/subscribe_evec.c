#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/evec.h>
#include <lotto/modules/evec/events.h>
#include <lotto/modules/yield/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

PS_ADVERTISE_TYPE(EVENT_EVEC_PREPARE)
PS_ADVERTISE_TYPE(EVENT_EVEC_WAIT)
PS_ADVERTISE_TYPE(EVENT_EVEC_TIMED_WAIT)
PS_ADVERTISE_TYPE(EVENT_EVEC_CANCEL)
PS_ADVERTISE_TYPE(EVENT_EVEC_WAKE)
PS_ADVERTISE_TYPE(EVENT_EVEC_MOVE)

void
intercept_evec_prepare(void *addr)
{
    evec_prepare_event_t ev = {.addr = addr};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_PREPARE, &ev, 0);
}

void
intercept_evec_wait(void *addr)
{
    evec_wait_event_t ev = {.addr = addr};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_WAIT, &ev, 0);
}

enum lotto_timed_wait_status
intercept_evec_timed_wait(void *addr, const struct timespec *restrict abstime)
{
    evec_timed_wait_event_t ev = {
        .addr    = addr,
        .abstime = abstime,
        .ret     = TIMED_WAIT_TIMEOUT,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_TIMED_WAIT, &ev, 0);
    return ev.ret;
}

void
intercept_evec_cancel(void *addr)
{
    evec_cancel_event_t ev = {.addr = addr};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_CANCEL, &ev, 0);
}

void
intercept_evec_wake(void *addr, uint32_t cnt)
{
    evec_wake_event_t ev = {
        .addr = addr,
        .cnt  = cnt,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_WAKE, &ev, 0);
}

void
intercept_evec_move(void *src, void *dst)
{
    evec_move_event_t ev = {
        .src = src,
        .dst = dst,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_EVEC_MOVE, &ev, 0);
}

void
_lotto_evec_prepare(void *addr)
{
    context_t ctx    = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_prepare_event_t ev = {.addr = addr};
    capture_point cp = {.src_type = EVENT_EVEC_PREPARE, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

void
_lotto_evec_wait(void *addr)
{
    context_t ctx = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_wait_event_t ev = {.addr = addr};
    capture_point cp = {.src_type = EVENT_EVEC_WAIT, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

enum lotto_timed_wait_status
_lotto_evec_timed_wait(void *addr, const struct timespec *restrict abstime)
{
    enum lotto_timed_wait_status ret;
    context_t ctx = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_timed_wait_event_t ev = {
        .addr    = addr,
        .abstime = abstime,
        .ret     = TIMED_WAIT_TIMEOUT,
    };
    capture_point cp = {.src_type = EVENT_EVEC_TIMED_WAIT, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
    ret = ev.ret;
    return ret;
}

void
_lotto_evec_cancel(void *addr)
{
    context_t ctx   = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_cancel_event_t ev = {.addr = addr};
    capture_point cp = {.src_type = EVENT_EVEC_CANCEL, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

void
_lotto_evec_wake(void *addr, uint32_t cnt)
{
    context_t ctx = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_wake_event_t ev = {
        .addr = addr,
        .cnt  = cnt,
    };
    capture_point cp = {.src_type = EVENT_EVEC_WAKE, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

void
_lotto_evec_move(void *src, void *dst)
{
    context_t ctx = *ctx(.self = self_md(), .func = __FUNCTION__);
    evec_move_event_t ev = {
        .src = src,
        .dst = dst,
    };
    capture_point cp = {.src_type = EVENT_EVEC_MOVE, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_PREPARE, {
    evec_prepare_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_evec_prepare(ev->addr);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_WAIT, {
    evec_wait_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_evec_wait(ev->addr);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_TIMED_WAIT, {
    evec_timed_wait_event_t *ev = EVENT_PAYLOAD(event);
    ev->ret = _lotto_evec_timed_wait(ev->addr, ev->abstime);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_CANCEL, {
    evec_cancel_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_evec_cancel(ev->addr);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_WAKE, {
    evec_wake_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_evec_wake(ev->addr, ev->cnt);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_EVEC_MOVE, {
    evec_move_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_evec_move(ev->src, ev->dst);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    switch (cp->src_type) {
        case EVENT_EVEC_PREPARE:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        case EVENT_EVEC_WAIT:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        case EVENT_EVEC_TIMED_WAIT:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        case EVENT_EVEC_CANCEL:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        case EVENT_EVEC_WAKE:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        case EVENT_EVEC_MOVE:
            runtime_ingress_module_submit_extra_event(origin, cp,
                                                      EVENT_SYS_YIELD);
            break;
        default:
            break;
    }
    return PS_OK;
})
