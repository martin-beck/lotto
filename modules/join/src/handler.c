#include <errno.h>
#include <pthread.h>

#define LOGGER_BLOCK LOGGER_CUR_BLOCK

#include <lotto/engine/dispatcher.h>
#include <lotto/engine/statemgr.h>
#include <lotto/modules/join/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/assert.h>
#include <lotto/sys/logger_block.h>
#include <lotto/util/macros.h>

/*******************************************************************************
 * ephemeral state
 ******************************************************************************/
typedef struct pthread_task_s {
    tiditem_t ti;
    uint64_t pthread;
    void *value_ptr;
    tidset_t waiters;
    bool terminated;
    bool detached;
} pthread_task_t;

typedef struct task_s {
    tiditem_t ti;
    task_id tid;
} task_t;

typedef struct waitee_s {
    tiditem_t ti;
    task_id waitee;
    void **value_ptr;
    int *ret;
} waitee_t;

typedef struct {
    tidmap_t pthreads;
    map_t tasks;
    tidmap_t waitees;
} state_t;

static state_t _state;

REGISTER_EPHEMERAL(_state, {
    tidmap_init(&_state.pthreads, MARSHABLE_STATIC(sizeof(pthread_task_t)));
    map_init(&_state.tasks, MARSHABLE_STATIC(sizeof(task_t)));
    tidmap_init(&_state.waitees, MARSHABLE_STATIC(sizeof(waitee_t)));
})

/*******************************************************************************
 * utils
 ******************************************************************************/
static void
_init(task_id tid, uint64_t pthread, bool detached)
{
    pthread_task_t *pt =
        (pthread_task_t *)tidmap_register(&_state.pthreads, tid);
    ASSERT(pt);
    pt->pthread   = pthread;
    pt->value_ptr = NULL;
    tidset_init(&pt->waiters);
    pt->terminated = false;
    pt->detached   = detached;
    task_t *t      = (task_t *)map_register(&_state.tasks, pthread);
    ASSERT(t);
    t->tid = tid;
}

static void
_join(task_id tid, uint64_t pthread, void **value_ptr, int *ret)
{
    task_t *t = (task_t *)map_find(&_state.tasks, pthread);
    if (t == NULL) {
        *ret = ESRCH;
        return;
    }
    task_id waitee = t->tid;
    waitee_t *wee;
    task_id waiteei = waitee;
    while (1) {
        if (waiteei == tid) {
            *ret = EDEADLK;
            return;
        }
        wee = (waitee_t *)tidmap_find(&_state.waitees, waiteei);
        if (wee == NULL) {
            break;
        }
        waiteei = wee->waitee;
    }
    pthread_task_t *pt =
        (pthread_task_t *)tidmap_find(&_state.pthreads, waitee);
    ASSERT(pt);
    if (pt->terminated) {
        if (value_ptr != NULL) {
            *value_ptr = pt->value_ptr;
        }
        *ret = 0;
        tidmap_deregister(&_state.pthreads, waitee);
        tidmap_deregister(&_state.tasks, pthread);
        return;
    }
    tidset_insert(&pt->waiters, tid);
    wee = (waitee_t *)tidmap_register(&_state.waitees, tid);
    ASSERT(wee);
    wee->waitee    = waitee;
    wee->value_ptr = value_ptr;
    wee->ret       = ret;
}

static void
_texit(task_id tid, void *value_ptr)
{
    pthread_task_t *pt = (pthread_task_t *)tidmap_find(&_state.pthreads, tid);
    ASSERT(pt);
    ASSERT(pt->value_ptr == NULL);
    pt->value_ptr = value_ptr;
}

static void
_fini(task_id tid)
{
    pthread_task_t *pt = (pthread_task_t *)tidmap_find(&_state.pthreads, tid);
    ASSERT(pt);
    if (pt->detached) {
        tidmap_deregister(&_state.tasks, pt->pthread);
        tidmap_deregister(&_state.pthreads, tid);
        return;
    }
    if (tidset_size(&pt->waiters) == 0) {
        pt->terminated = true;
        return;
    }
    for (size_t i = 0; i < tidset_size(&pt->waiters); i++) {
        task_id waiter = tidset_get(&pt->waiters, i);
        waitee_t *wee  = (waitee_t *)tidmap_find(&_state.waitees, waiter);
        ASSERT(wee);
        if (wee->value_ptr) {
            *wee->value_ptr = pt->value_ptr;
        }
        *wee->ret = 0;
        tidmap_deregister(&_state.waitees, waiter);
    }
    tidmap_deregister(&_state.tasks, pt->pthread);
    tidmap_deregister(&_state.pthreads, tid);
}

static void
_detach(task_id tid, uint64_t pthread, int *ret)
{
    task_t *t = (task_t *)map_find(&_state.tasks, pthread);
    if (t == NULL) {
        *ret = ESRCH;
        return;
    }
    task_id id        = t->tid;
    pthread_task_t *p = (pthread_task_t *)tidmap_find(&_state.pthreads, id);
    ASSERT(p != NULL);
    if (p->detached) {
        *ret = EINVAL;
        return;
    }
    p->detached = true;
    ASSERT(tidmap_find(&_state.waitees, id) == NULL);
    *ret = 0;
}

bool
_should_wait(task_id id)
{
    return tidmap_find(&_state.waitees, id) != NULL;
}

static uintptr_t
_join_thread(const context_t *ctx)
{
    if (context_has_capture_point(ctx)) {
        switch (ctx->src_type) {
            case EVENT_JOIN:
                return ((join_event_t *)ctx->cp->payload)->thread;
            case EVENT_TASK_INIT:
                return context_task_init_thread(ctx);
            case EVENT_TASK_DETACH:
                return context_task_detach_thread(ctx);
            default:
                break;
        }
    }
    return (uintptr_t)ctx->args[0].value.u64;
}

static bool
_join_detached(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_INIT) {
        return context_task_init_detached(ctx);
    }
    return ctx->args[1].value.u8;
}

static void **
_join_value_ptr(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_JOIN) {
        return ((join_event_t *)ctx->cp->payload)->ptr;
    }
    return (void **)ctx->args[1].value.ptr;
}

static int *
_join_ret(const context_t *ctx)
{
    if (context_has_capture_point(ctx)) {
        switch (ctx->src_type) {
            case EVENT_JOIN:
                return ((join_event_t *)ctx->cp->payload)->ret;
            case EVENT_TASK_DETACH:
                return context_task_detach_ret(ctx);
            default:
                break;
        }
    }
    if (ctx->cat == CAT_DETACH) {
        return (int *)ctx->args[1].value.ptr;
    }
    return (int *)ctx->args[2].value.ptr;
}

static void *
_join_exit_value(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_TASK_FINI) {
        return context_task_fini_value(ctx);
    }
    return (void *)ctx->args[0].value.ptr;
}

/*******************************************************************************
 * handler
 ******************************************************************************/
STATIC void
_join_handle(const context_t *ctx, event_t *e)
{
    ASSERT(ctx);
    ASSERT(ctx->id != NO_TASK);
    ASSERT(e);

    switch (ctx->cat) {
        case CAT_TASK_INIT:
            _init(ctx->id, _join_thread(ctx), _join_detached(ctx));
            break;
        case CAT_JOIN:
            _join(ctx->id, _join_thread(ctx), _join_value_ptr(ctx),
                  _join_ret(ctx));
            break;
        case CAT_DETACH:
            _detach(ctx->id, _join_thread(ctx), _join_ret(ctx));
            break;
        case CAT_EXIT:
            _texit(ctx->id, _join_exit_value(ctx));
            break;
        case CAT_TASK_FINI:
            _fini(ctx->id);
            break;
        default:
            break;
    }
    if (e->skip) {
        return;
    }
    tidset_remove_all_keys(&e->tset, &_state.waitees);
    if (_should_wait(ctx->id)) {
        e->is_chpt = true;
        ASSERT(e->any_task_filter == NULL);
        e->any_task_filter = _should_wait;
    }
}
REGISTER_SEQUENCER_HANDLER(_join_handle)
