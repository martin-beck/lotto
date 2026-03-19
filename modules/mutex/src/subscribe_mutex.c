#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/mutex.h>
#include <lotto/modules/mutex/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

PS_ADVERTISE_TYPE(EVENT_MUTEX_ACQUIRE)
PS_ADVERTISE_TYPE(EVENT_MUTEX_TRYACQUIRE)
PS_ADVERTISE_TYPE(EVENT_MUTEX_RELEASE)

void
intercept_mutex_acquire_named(const char *func, void *addr, const void *pc)
{
    mutex_acquire_event_t ev = {
        .pc   = pc,
        .func = func,
        .addr = addr,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_MUTEX_ACQUIRE, &ev, 0);
}

void
intercept_mutex_acquire(void *addr, const void *pc)
{
    intercept_mutex_acquire_named(__FUNCTION__, addr, pc);
}

int
intercept_mutex_tryacquire_named(const char *func, void *addr, const void *pc)
{
    mutex_tryacquire_event_t ev = {
        .pc   = pc,
        .func = func,
        .addr = addr,
        .ret  = 0,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_MUTEX_TRYACQUIRE, &ev, 0);
    return ev.ret;
}

int
intercept_mutex_tryacquire(void *addr, const void *pc)
{
    return intercept_mutex_tryacquire_named(__FUNCTION__, addr, pc);
}

void
intercept_mutex_release_named(const char *func, void *addr, const void *pc)
{
    mutex_release_event_t ev = {
        .pc   = pc,
        .func = func,
        .addr = addr,
    };
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_MUTEX_RELEASE, &ev, 0);
}

void
intercept_mutex_release(void *addr, const void *pc)
{
    intercept_mutex_release_named(__FUNCTION__, addr, pc);
}

void
_lotto_mutex_acquire_named_src(type_id src_type, const char *func, void *addr,
                               const void *pc)
{
    context_t ctx                  = *ctx_pc(.self = self_md(),
                            .pc = (uintptr_t)pc, .func = func);
    mutex_acquire_event_t ev = {
        .func = func,
        .addr = addr,
        .pc   = pc,
    };
    capture_point cp = {.src_type = src_type, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
}

void
_lotto_mutex_acquire_named(const char *func, void *addr, const void *pc)
{
    _lotto_mutex_acquire_named_src(EVENT_MUTEX_ACQUIRE, func, addr, pc);
}
void
_lotto_mutex_acquire(void *addr, const void *pc)
{
    _lotto_mutex_acquire_named(__FUNCTION__, addr, pc);
}


int
_lotto_mutex_tryacquire_named_src(type_id src_type, const char *func, void *addr,
                                  const void *pc)
{
    int ret                        = 0;
    context_t ctx                  = *ctx_pc(.self = self_md(),
                            .pc = (uintptr_t)pc, .func = func);
    mutex_tryacquire_event_t ev = {
        .func = func,
        .addr = addr,
        .pc   = pc,
        .ret  = ret,
    };
    capture_point cp = {.src_type = src_type, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return ev.ret;
}

int
_lotto_mutex_tryacquire_named(const char *func, void *addr, const void *pc)
{
    return _lotto_mutex_tryacquire_named_src(EVENT_MUTEX_TRYACQUIRE, func, addr,
                                             pc);
}

int
_lotto_mutex_tryacquire(void *addr, const void *pc)
{
    return _lotto_mutex_tryacquire_named(__FUNCTION__, addr, pc);
}

void
_lotto_mutex_release_named_src(type_id src_type, const char *func, void *addr,
                               const void *pc)
{
    context_t ctx                  = *ctx_pc(.self = self_md(),
                            .pc = (uintptr_t)pc, .func = func);
    mutex_release_event_t ev = {
        .func = func,
        .addr = addr,
        .pc   = pc,
    };
    capture_point cp = {.src_type = src_type, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
}

void
_lotto_mutex_release_named(const char *func, void *addr, const void *pc)
{
    _lotto_mutex_release_named_src(EVENT_MUTEX_RELEASE, func, addr, pc);
}

void
_lotto_mutex_release(void *addr, const void *pc)
{
    _lotto_mutex_release_named(__FUNCTION__, addr, pc);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MUTEX_ACQUIRE, {
    mutex_acquire_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_mutex_acquire_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MUTEX_TRYACQUIRE, {
    mutex_tryacquire_event_t *ev = EVENT_PAYLOAD(event);
    ev->ret = _lotto_mutex_tryacquire_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MUTEX_RELEASE, {
    mutex_release_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_mutex_release_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;

    switch (cp->src_type) {
        case EVENT_MUTEX_ACQUIRE: {
            runtime_ingress_module_submit_auto(origin, cp);
            break;
        }
        case EVENT_MUTEX_TRYACQUIRE: {
            runtime_ingress_module_submit_auto(origin, cp);
            break;
        }
        case EVENT_MUTEX_RELEASE: {
            runtime_ingress_module_submit_auto(origin, cp);
            break;
        }
        default:
            break;
    }
    return PS_OK;
})
