#include <pthread.h>
#include <stdbool.h>

#define LOGGER_PREFIX LOGGER_CUR_FILE
#include "sighandler.h"
#include <lotto/base/context.h>
#include <lotto/check.h>
#include <lotto/engine/catmgr.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_origin.h>
#include <lotto/runtime/context_payload.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/mediator.h>
#include <lotto/runtime/runtime.h>
#include <lotto/sys/assert.h>
#include <lotto/sys/ensure.h>
#include <lotto/sys/real.h>
#include <lotto/util/macros.h>


// Prevent intercepting accesses before lotto is ready.
static bool _lotto_initialized = false;
static void _intercept_return_resume(mediator_t *m, context_t *ctx);
static void _intercept_resume(mediator_t *m, context_t *ctx);
static void _assert_runtime_ingress_event(type_id type,
                                          const capture_point *cp);
static void _runtime_ingress_capture_with(context_t *ctx);
static mediator_t *_runtime_ingress_before_with(context_t *ctx);
static void _runtime_ingress_after_with(context_t *ctx);

bool
_lotto_loaded(void)
{
    /* Inform user that the Lotto runtime library is loaded. */
    return true;
}

bool
lotto_intercept_initialized(void)
{
#if defined(LOTTO_TEST)
    return true;
#else /* LOTTO_TEST */
    return _lotto_initialized;
#endif
}

void
lotto_set_interceptor_initialized(void)
{
    ASSERT(_lotto_initialized == false);
    _lotto_initialized = true;
}

void
interceptor_fini(const context_t *ctx)
{
    mediator_fini(get_existing_mediator());
}

#define MATCH_NAME(A, B) (strcmp(A, #B) == 0)

static bool
_is_task_create(const char *func, bool after)
{
    return MATCH_NAME(func, pthread_create) ||
           MATCH_NAME(func, _ZNSt6thread15_M_start_thread);
    mediator_fini(get_mediator(false));
}

/*******************************************************************************
 * task initialization and termination
 ******************************************************************************/
mediator_t *
get_mediator(bool new_task)
{
    mediator_t *m = mediator_get_data(new_task);
    if (m->registration_status != MEDIATOR_REGISTRATION_NEED) {
        return m;
    }
    m->registration_status = MEDIATOR_REGISTRATION_EXEC;
    logger_debugf("[%lu] register new task\n", m->id);
    context_t *ctx           = ctx(.func = __FUNCTION__);
    mediator_status_t status = mediator_resume(m, ctx);
    ASSERT((status == MEDIATOR_OK) && "mediator_init failed");
    if (!new_task) {
        /* take over task initialization */
        *ctx = runtime_context_synthetic(__FUNCTION__, EVENT_TASK_INIT);
        ENSURE(!mediator_capture(m, ctx) &&
               "expected mediator_capture to return false");
        _intercept_resume(m, ctx);
    }
    m->registration_status = MEDIATOR_REGISTRATION_DONE;
    return m;
}

mediator_t *
get_existing_mediator(void)
{
    return mediator_get_existing_data();
}

static void
_intercept_resume(mediator_t *m, context_t *ctx)
{
    logger_debugf("[%lu] prepare to resume %s\n", m->id,
                  category_str(context_effective_category(ctx)));

    switch (mediator_resume(m, ctx)) {
        case MEDIATOR_OK:
            break;
        case MEDIATOR_ABORT:
            lotto_exit(ctx, REASON_ABORT);
            sys_abort();
            break;
        case MEDIATOR_SHUTDOWN:
            lotto_exit(ctx, REASON_SHUTDOWN);
            sys_abort();
            break;
        default:
            logger_fatalf("unexpected mediator resume output");
            break;
    };
}

static void
_intercept_return_resume(mediator_t *m, context_t *ctx)
{
    ASSERT(lotto_intercept_initialized());

    logger_debugf("[%lu] return from '%s'\n", m->id, ctx->func);
    mediator_return(m, ctx);
    _intercept_resume(m, ctx);
}

/*******************************************************************************
 * public interface
 ******************************************************************************/

// normalized runtime ingress
void
runtime_ingress(context_t *ctx)
{
    if (!lotto_intercept_initialized())
        return;

    mediator_t *m = get_mediator(context_is_task_init(ctx));

    if (!mediator_capture(m, ctx))
        _intercept_resume(m, ctx);
}

static void
_runtime_ingress_capture_with(context_t *ctx)
{
    runtime_ingress(ctx);
}

void
runtime_ingress_capture(const ingress_capture *capture)
{
    context_t ctx = runtime_context_from_ingress_capture(capture);
    _runtime_ingress_capture_with(&ctx);
}

void
runtime_ingress_event(const context_origin *origin, type_id type,
                      const capture_point *cp)
{
    ingress_capture capture =
        runtime_ingress_capture_base(origin, type, cp);
    _assert_runtime_ingress_event(type, cp);
    runtime_ingress_capture(&capture);
}


// normalized runtime ingress for blocking calls
mediator_t *
runtime_ingress_before(context_t *ctx)
{
    if (!lotto_intercept_initialized()) {
        logger_debugf(
            "[???] before call '%s' (interceptor not initialized yet)\n",
            ctx->func);
        return NULL;
    }
    mediator_t *m = get_mediator(false);
    logger_debugf("[%lu] before call '%s'\n", m->id, ctx->func);
    ENSURE(mediator_capture(m, ctx));
    return m;
}

static mediator_t *
_runtime_ingress_before_with(context_t *ctx)
{
    return runtime_ingress_before(ctx);
}

mediator_t *
runtime_ingress_capture_before(const ingress_capture *capture)
{
    ASSERT(capture->phase == CONTEXT_PHASE_BEFORE);
    context_t ctx = runtime_context_from_ingress_capture(capture);
    return _runtime_ingress_before_with(&ctx);
}

mediator_t *
runtime_ingress_event_before(const context_origin *origin, type_id type,
                             const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_capture_base_phase(
        origin, type, cp, CONTEXT_PHASE_BEFORE);
    _assert_runtime_ingress_event(type, cp);
    return runtime_ingress_capture_before(&capture);
}

void
runtime_ingress_after(context_t *ctx)
{
    if (!lotto_intercept_initialized()) {
        logger_debugf("[?] after call '%s' (interceptor not initialized yet)\n",
                      ctx->func);
        return;
    }
    mediator_t *m = get_mediator(false);

    logger_debugf("[%lu] after call  '%s'\n", m->id, ctx->func);
    _intercept_return_resume(m, ctx);
}

static void
_runtime_ingress_after_with(context_t *ctx)
{
    runtime_ingress_after(ctx);
}

void
runtime_ingress_capture_after(const ingress_capture *capture)
{
    ASSERT(capture->phase == CONTEXT_PHASE_AFTER);
    context_t ctx = runtime_context_from_ingress_capture(capture);
    _runtime_ingress_after_with(&ctx);
}

void
runtime_ingress_event_after(const context_origin *origin, type_id type,
                            const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_capture_base_phase(
        origin, type, cp, CONTEXT_PHASE_AFTER);
    _assert_runtime_ingress_event(type, cp);
    runtime_ingress_capture_after(&capture);
}

void _lotto_enable_unregistered();

void *
intercept_lookup_call(const char *func)
{
    void *foo = real_func(func, 0);
    if (!lotto_intercept_initialized()) {
        return foo;
    }

    context_t *ctx = ctx_empty;
    *ctx           = runtime_context_synthetic(func, EVENT_CALL);
    (void)runtime_ingress_before(ctx);

    logger_debugf("[%lu] lookup call '%s'\n", ctx->id, func);
    /* search for real function and return its pointer */
    if (foo == NULL)
        logger_fatalf("could not find function '%s'\n", func);
    logger_debugf("[%lu] found function '%s'\n", ctx->id, func);
    return foo;
}

void *
intercept_warn_call(const char *func)
{
    /* search for the real function and return its pointer */
    void *foo = real_func(func, 0);
    if (!lotto_intercept_initialized()) {
        return foo;
    }

    logger_warnf("warn call '%s'\n", func);
    if (foo == NULL)
        logger_fatalf("could not find function '%s'\n", func);
    return foo;
}

static fini_t _fini[MAX_FINI];
static int _fini_cnt = 0;

void
lotto_intercept_register_fini(fini_t func)
{
    ASSERT(_fini_cnt < MAX_FINI);
    _fini[_fini_cnt++] = func;
}

void
lotto_intercept_fini()
{
    for (int i = 0; i < _fini_cnt; ++i) {
        _fini[i]();
    }
}

static void
_assert_runtime_ingress_event(type_id type, const capture_point *cp)
{
    ASSERT(cp != NULL);

    switch (type) {
        case EVENT_KEY_CREATE:
            ASSERT(cp->key_create != NULL);
            break;
        case EVENT_TASK_INIT:
            ASSERT(cp->task_init != NULL);
            break;
        case EVENT_TASK_FINI:
            ASSERT(cp->task_fini != NULL);
            break;
        case EVENT_TASK_CREATE:
            ASSERT(cp->task_create != NULL || cp->payload == NULL);
            break;
        case EVENT_CALL:
        case EVENT_TASK_BLOCK:
            break;
        case EVENT_TASK_DETACH:
            ASSERT(cp->task_detach != NULL);
            break;
        case EVENT_KEY_DELETE:
            ASSERT(cp->key_delete != NULL);
            break;
        case EVENT_SET_SPECIFIC:
            ASSERT(cp->set_specific != NULL);
            break;
        default:
            logger_fatalf("unexpected ingress event type: %u\n", type);
            break;
    }
}
