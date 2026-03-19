/**
 * @file ingress.h
 * @brief Runtime declarations for ingress.
 */
#ifndef LOTTO_INGRESS_H
#define LOTTO_INGRESS_H

#include <lotto/base/context.h>
#include <lotto/base/task_id.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/mediator.h>
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
void runtime_ingress_event(const context_t *origin, type_id type,
                           const capture_point *cp);

/* called before executing an asm-intercepted external function. */
void *intercept_lookup_call(const char *func);

/* called before executing an intercepted function with known context.
 */
mediator_t *runtime_ingress_before(context_t *ctx);
mediator_t *runtime_ingress_event_before(const context_t *origin, type_id type,
                                         const capture_point *cp);

/* called after executing an intercepted external function. */
void runtime_ingress_after(context_t *ctx);
void runtime_ingress_event_after(const context_t *origin, type_id type,
                                 const capture_point *cp);

static inline context_t
runtime_ingress_module_base(const context_t *origin, const capture_point *cp)
{
    ASSERT(origin != NULL);
    ASSERT(cp != NULL);
    context_t ctx = *origin;
    ctx.type      = EVENT_MODULE_INTERCEPT;
    ctx.src_type  = cp->src_type;
    ctx.cp        = (capture_point *)cp;
    return ctx;
}

static inline context_t
runtime_ingress_module_context(const context_t *origin, const capture_point *cp,
                               category_t cat)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ctx.cat       = cat;
    return ctx;
}

static inline void
runtime_ingress_module_submit_args(const context_t *origin,
                                   const capture_point *cp, category_t cat,
                                   arg_t arg0, arg_t arg1, arg_t arg2,
                                   arg_t arg3)
{
    context_t ctx = runtime_ingress_module_context(origin, cp, cat);
    ctx.args[0]   = arg0;
    ctx.args[1]   = arg1;
    ctx.args[2]   = arg2;
    ctx.args[3]   = arg3;
    runtime_ingress(&ctx);
}

static inline void
runtime_ingress_module_submit(const context_t *origin, const capture_point *cp,
                              category_t cat)
{
    runtime_ingress_module_submit_args(origin, cp, cat, (arg_t){0}, (arg_t){0},
                                       (arg_t){0}, (arg_t){0});
}

static inline mediator_t *
runtime_ingress_module_submit_before_args(const context_t *origin,
                                          const capture_point *cp,
                                          category_t cat, arg_t arg0,
                                          arg_t arg1, arg_t arg2, arg_t arg3)
{
    context_t ctx = runtime_ingress_module_context(origin, cp, cat);
    ctx.args[0]   = arg0;
    ctx.args[1]   = arg1;
    ctx.args[2]   = arg2;
    ctx.args[3]   = arg3;
    return runtime_ingress_before(&ctx);
}

static inline mediator_t *
runtime_ingress_module_submit_before(const context_t *origin,
                                     const capture_point *cp, category_t cat)
{
    return runtime_ingress_module_submit_before_args(
        origin, cp, cat, (arg_t){0}, (arg_t){0}, (arg_t){0}, (arg_t){0});
}

static inline void
runtime_ingress_module_submit_after(const context_t *origin,
                                    const capture_point *cp, category_t cat)
{
    context_t ctx = runtime_ingress_module_context(origin, cp, cat);
    runtime_ingress_after(&ctx);
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
