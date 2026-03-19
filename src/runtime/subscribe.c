#include <errno.h>
#include <pthread.h>
#include <stdint.h>

#ifdef DICE_MODULE_SLOT
    #undef DICE_MODULE_SLOT
#endif
#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/events/pthread.h>
#include <dice/events/self.h>
#include <dice/events/thread.h>
#include <dice/interpose.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <dice/self.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_origin.h>
#include <lotto/runtime/events.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/logger.h>

// -----------------------------------------------------------------------------
// thread_start and thread_exit
// -----------------------------------------------------------------------------

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_SELF_INIT, {
    if (self_id(md) != MAIN_THREAD)
        return PS_OK;
    bool detached = false;
    context_origin ctx =
        *ctx_origin(.self = md, .func = "pthread_thread_start");
    capture_task_init_event ev = {
        .thread   = (uintptr_t)pthread_self(),
        .detached = detached,
    };
    capture_point cp = {.src_type = EVENT_TASK_INIT, .task_init = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_TASK_INIT, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_THREAD_START, {
    bool detached = false;
    context_origin ctx =
        *ctx_origin(.self = md, .func = "pthread_thread_start");
    capture_task_init_event ev = {
        .thread   = (uintptr_t)pthread_self(),
        .detached = detached,
    };
    capture_point cp = {.src_type = EVENT_TASK_INIT, .task_init = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_TASK_INIT, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_THREAD_EXIT, {
    struct pthread_exit_event *ev = EVENT_PAYLOAD(event);
    context_origin ctx = *ctx_origin(.self = md, .func = "pthread_exit");
    capture_task_fini_event fev = {.ptr = ev != NULL ? ev->ptr : NULL};
    capture_point cp = {.src_type = EVENT_TASK_FINI, .task_fini = &fev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_TASK_FINI, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

// -----------------------------------------------------------------------------
// pthread_create and pthread_join
// -----------------------------------------------------------------------------

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_THREAD_CREATE, {
    struct pthread_create_event *ev = EVENT_PAYLOAD(event);
    context_origin ctx = *ctx_origin(.self = md, .func = "pthread_create");
    capture_task_create_event cev = {
        .thread = ev->thread,
        .attr   = ev->attr,
        .run    = ev->run,
    };
    capture_point cp = {.src_type = EVENT_TASK_CREATE, .task_create = &cev};
    PS_PUBLISH(CHAIN_INGRESS_BEFORE, EVENT_TASK_CREATE, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_THREAD_CREATE, {
    context_origin ctx = *ctx_origin(.self = md, .func = "pthread_create");
    capture_point cp   = {.src_type = EVENT_TASK_CREATE, .payload = NULL};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_TASK_CREATE, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_THREAD_JOIN, {
    context_origin ctx = *ctx_origin(.self = md, .func = "pthread_join");
    capture_point cp   = {.src_type = EVENT_CALL, .payload = NULL};
    PS_PUBLISH(CHAIN_INGRESS_BEFORE, EVENT_CALL, &cp, (metadata_t *)&ctx);
})
PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_THREAD_JOIN, {
    context_origin ctx = *ctx_origin(.self = md, .func = "pthread_join");
    capture_point cp   = {.src_type = EVENT_CALL, .payload = NULL};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_CALL, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

struct pthread_detach_event {
    const void *pc;
    pthread_t thread;
    void (*func)(pthread_t);
    int ret;
};

LOTTO_ADVERTISE_TYPE(EVENT_PTHREAD_DETACH)

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_PTHREAD_DETACH, {
    struct pthread_detach_event *ev = EVENT_PAYLOAD(event);

    int ret            = EINTR;
    context_origin ctx = *ctx_origin_pc(.self = md, .pc = (uintptr_t)ev->pc,
                                        .func = __FUNCTION__);
    capture_task_detach_event dev = {
        .thread = ev->thread,
        .ret    = &ret,
    };
    capture_point cp = {.src_type = EVENT_TASK_DETACH, .task_detach = &dev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_TASK_DETACH, &cp, (metadata_t *)&ctx);
    ASSERT(ret != EINTR);
    return PS_OK;
})

#if 0
INTERPOSE(int, pthread_detach, pthread_t thread)
{
    struct pthread_detach_event ev = {
        .pc     = INTERPOSE_PC,
        .thread = thread,
        .func   = REAL_FUNC(pthread_detach),
        .ret    = 0,
    };

    struct metadata md = {0};
    PS_PUBLISH(INTERCEPT_BEFORE, EVENT_PTHREAD_DETACH, &ev, &md);
    ev.ret = ev.func(ev.thread);
    PS_PUBLISH(INTERCEPT_AFTER, EVENT_PTHREAD_DETACH, &ev, &md);
    return ev.ret;
}
#endif
