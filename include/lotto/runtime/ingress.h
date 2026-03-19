/**
 * @file ingress.h
 * @brief Runtime declarations for ingress.
 */
#ifndef LOTTO_INGRESS_H
#define LOTTO_INGRESS_H

#include <dice/events/pthread.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/base/task_id.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_origin.h>
#include <lotto/runtime/context_payload.h>
#include <lotto/runtime/ingress_capture.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/mediator.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/util/macros.h>

/*******************************************************************************
 * mediator's task_id (in TLS)
 ******************************************************************************/

static inline task_id
get_task_id(void)
{
    mediator_t *m = mediator_get_data(false);
    ASSERT((!!m) && "thread-specific mediator data not initialized");
    return (m->id);
}

/*******************************************************************************
 * guarded lotto_step
 ******************************************************************************/

void runtime_ingress(context_t *ctx);
void runtime_ingress_capture(const ingress_capture *capture);
void runtime_ingress_event(const context_origin *origin, type_id type,
                           const capture_point *cp);

/* called before executing an asm-intercepted external function. */
void *intercept_lookup_call(const char *func);

/* called before executing an intercepted function with known context.
 */
mediator_t *runtime_ingress_before(context_t *ctx);
mediator_t *runtime_ingress_capture_before(const ingress_capture *capture);
mediator_t *runtime_ingress_event_before(const context_origin *origin,
                                         type_id type, const capture_point *cp);

/* called after executing an intercepted external function. */
void runtime_ingress_after(context_t *ctx);
void runtime_ingress_capture_after(const ingress_capture *capture);
void runtime_ingress_event_after(const context_origin *origin, type_id type,
                                 const capture_point *cp);

static inline ingress_capture
runtime_ingress_module_capture_base(const context_origin *origin,
                                    const capture_point *cp)
{
    ASSERT(cp != NULL);
    return runtime_ingress_capture_base(origin, cp->src_type, cp);
}

static inline ingress_capture
runtime_ingress_module_capture_auto(const context_origin *origin,
                                    const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_module_capture_base(origin, cp);
    ASSERT(context_event_category(capture.type) != CAT_NONE ||
           context_event_category(capture.src_type) != CAT_NONE);
    return capture;
}

static inline void
runtime_ingress_module_submit_auto(const context_origin *origin,
                                   const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_module_capture_auto(origin, cp);
    runtime_ingress_capture(&capture);
}

static inline void
runtime_ingress_module_submit_event(const context_origin *origin,
                                    const capture_point *cp, type_id type)
{
    ingress_capture capture =
        runtime_ingress_capture_base(origin, type, cp);
    runtime_ingress_capture(&capture);
}

static inline mediator_t *
runtime_ingress_module_submit_before_auto(const context_origin *origin,
                                          const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_module_capture_auto(origin, cp);
    return runtime_ingress_capture_before(&capture);
}

static inline void
runtime_ingress_module_submit_after_auto(const context_origin *origin,
                                         const capture_point *cp)
{
    ingress_capture capture = runtime_ingress_module_capture_auto(origin, cp);
    runtime_ingress_capture_after(&capture);
}

static inline void
runtime_ingress_module_submit_extra_event(const context_origin *origin,
                                          const capture_point *cp,
                                          type_id extra_type)
{
    runtime_ingress_module_submit_event(origin, cp, extra_type);
    runtime_ingress_module_submit_auto(origin, cp);
}

static inline ingress_capture
runtime_ingress_memaccess_capture(const context_origin *origin,
                                  const capture_point *cp, bool after)
{
    context_memaccess_event_t event = context_memaccess_event_from_source(
        cp->src_type, cp->payload,
        after ? CONTEXT_PHASE_AFTER : CONTEXT_PHASE_BEFORE);
    ingress_capture capture = runtime_ingress_module_capture_base(origin, cp);
    capture.phase = after ? CONTEXT_PHASE_AFTER : CONTEXT_PHASE_BEFORE;
    capture.type  = context_memaccess_type_from_event(event);
    ASSERT(capture.type != 0);
    return capture;
}

static inline void
runtime_ingress_module_submit_memaccess(const context_origin *origin,
                                        const capture_point *cp)
{
    ingress_capture capture =
        runtime_ingress_memaccess_capture(origin, cp, false);
    runtime_ingress_capture(&capture);
}

static inline void
runtime_ingress_module_submit_memaccess_after(const context_origin *origin,
                                              const capture_point *cp)
{
    ingress_capture capture =
        runtime_ingress_memaccess_capture(origin, cp, true);
    runtime_ingress_capture(&capture);
}

/*
 * Return the mediator of the current task, init flag also initializes
 * mediator if not yet.
 */
mediator_t *get_mediator(bool new_task);

/*
 * Like get_mediator but does not initialize mediator if not yet.
 */
mediator_t *get_existing_mediator();

/// true if lotto interceptors have been initialized
bool lotto_intercept_initialized(void);

void *intercept_warn_call(const char *func);

#define MAX_FINI 100
typedef void (*fini_t)();
void lotto_intercept_register_fini(fini_t func);
void lotto_intercept_fini();

#endif
