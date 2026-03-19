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

struct mutex_acquire_event {
    const void *pc;
    const char *func;
    void *addr;
};

struct mutex_tryacquire_event {
    const void *pc;
    const char *func;
    void *addr;
    int ret;
};

struct mutex_release_event {
    const void *pc;
    const char *func;
    void *addr;
};

PS_ADVERTISE_TYPE(EVENT_MUTEX_ACQUIRE)
PS_ADVERTISE_TYPE(EVENT_MUTEX_TRYACQUIRE)
PS_ADVERTISE_TYPE(EVENT_MUTEX_RELEASE)

void
intercept_mutex_acquire_named(const char *func, void *addr, const void *pc)
{
    struct mutex_acquire_event ev = {
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
    struct mutex_tryacquire_event ev = {
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
    struct mutex_release_event ev = {
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
    struct mutex_acquire_event ev = {
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
    struct mutex_tryacquire_event ev = {
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
    struct mutex_release_event ev = {
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
    struct mutex_acquire_event *ev = EVENT_PAYLOAD(event);
    _lotto_mutex_acquire_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MUTEX_TRYACQUIRE, {
    struct mutex_tryacquire_event *ev = EVENT_PAYLOAD(event);
    ev->ret = _lotto_mutex_tryacquire_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MUTEX_RELEASE, {
    struct mutex_release_event *ev = EVENT_PAYLOAD(event);
    _lotto_mutex_release_named(ev->func, ev->addr, ev->pc);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = *origin;

    switch (cp->src_type) {
        case EVENT_MUTEX_ACQUIRE: {
            struct mutex_acquire_event *ev = cp->payload;
            ctx.cat                       = CAT_MUTEX_ACQUIRE;
            ctx.args[0]                   = arg_ptr(ev->addr);
            runtime_ingress(&ctx);
            break;
        }
        case EVENT_MUTEX_TRYACQUIRE: {
            struct mutex_tryacquire_event *ev = cp->payload;
            ctx.cat                          = CAT_MUTEX_TRYACQUIRE;
            ctx.args[0]                      = arg_ptr(ev->addr);
            runtime_ingress(&ctx);
            ev->ret = (int)ctx.args[1].value.u8;
            break;
        }
        case EVENT_MUTEX_RELEASE: {
            struct mutex_release_event *ev = cp->payload;
            ctx.cat                       = CAT_MUTEX_RELEASE;
            ctx.args[0]                   = arg_ptr(ev->addr);
            runtime_ingress(&ctx);
            break;
        }
        default:
            break;
    }
    return PS_OK;
})
