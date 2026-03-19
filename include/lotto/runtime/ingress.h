/**
 * @file ingress.h
 * @brief Runtime declarations for ingress.
 */
#ifndef LOTTO_INGRESS_H
#define LOTTO_INGRESS_H

#include <lotto/base/context.h>
#include <lotto/base/task_id.h>
#include <lotto/base/category.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/runtime/mediator.h>
#include <lotto/runtime/module_event_category.h>
#include <lotto/util/macros.h>

#include <dice/events/pthread.h>

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
runtime_ingress_finalize_context(context_t ctx, category_t fallback_cat)
{
    return context_finalize_category(ctx, fallback_cat);
}

static inline context_t
runtime_ingress_module_base(const context_t *origin, const capture_point *cp)
{
    ASSERT(origin != NULL);
    ASSERT(cp != NULL);
    context_t ctx = *origin;
    ctx.type      = cp->src_type;
    ctx.src_type  = cp->src_type;
    ctx.cp        = (capture_point *)cp;
    return runtime_ingress_finalize_context(ctx, CAT_NONE);
}

static inline context_t
runtime_ingress_module_context(const context_t *origin, const capture_point *cp,
                               category_t cat)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ctx.cat       = cat;
    return runtime_ingress_finalize_context(ctx, cat);
}

static inline context_t
runtime_ingress_module_context_auto(const context_t *origin,
                                    const capture_point *cp)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ASSERT(ctx.cat != CAT_NONE);
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

static inline void
runtime_ingress_module_submit_auto(const context_t *origin,
                                   const capture_point *cp)
{
    context_t ctx = runtime_ingress_module_context_auto(origin, cp);
    runtime_ingress(&ctx);
}

static inline void
runtime_ingress_module_submit_event(const context_t *origin,
                                    const capture_point *cp, type_id type)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ctx.type      = type;
    ctx.cat       = CAT_NONE;
    ctx           = runtime_ingress_finalize_context(ctx, CAT_NONE);
    runtime_ingress(&ctx);
}

static inline void
runtime_ingress_module_submit_event_args(const context_t *origin,
                                         const capture_point *cp, type_id type,
                                         arg_t arg0, arg_t arg1, arg_t arg2,
                                         arg_t arg3)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ctx.type      = type;
    ctx.args[0]   = arg0;
    ctx.args[1]   = arg1;
    ctx.args[2]   = arg2;
    ctx.args[3]   = arg3;
    ctx.cat       = CAT_NONE;
    ctx           = runtime_ingress_finalize_context(ctx, CAT_NONE);
    runtime_ingress(&ctx);
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

static inline mediator_t *
runtime_ingress_module_submit_before_auto(const context_t *origin,
                                          const capture_point *cp)
{
    context_t ctx = runtime_ingress_module_context_auto(origin, cp);
    return runtime_ingress_before(&ctx);
}

static inline mediator_t *
runtime_ingress_module_submit_before_auto_args(const context_t *origin,
                                               const capture_point *cp,
                                               arg_t arg0, arg_t arg1,
                                               arg_t arg2, arg_t arg3)
{
    context_t ctx = runtime_ingress_module_context_auto(origin, cp);
    ctx.args[0]   = arg0;
    ctx.args[1]   = arg1;
    ctx.args[2]   = arg2;
    ctx.args[3]   = arg3;
    return runtime_ingress_before(&ctx);
}

static inline void
runtime_ingress_module_submit_after(const context_t *origin,
                                    const capture_point *cp, category_t cat)
{
    context_t ctx = runtime_ingress_module_context(origin, cp, cat);
    runtime_ingress_after(&ctx);
}

static inline void
runtime_ingress_module_submit_after_dynamic(const context_t *origin,
                                            const capture_point *cp,
                                            category_t cat)
{
    runtime_ingress_module_submit_after(origin, cp, cat);
}

static inline void
runtime_ingress_module_submit_after_auto(const context_t *origin,
                                         const capture_point *cp)
{
    context_t ctx = runtime_ingress_module_context_auto(origin, cp);
    runtime_ingress_after(&ctx);
}

static inline void
runtime_ingress_module_submit_extra_event(const context_t *origin,
                                          const capture_point *cp,
                                          type_id extra_type)
{
    runtime_ingress_module_submit_event(origin, cp, extra_type);
    runtime_ingress_module_submit_auto(origin, cp);
}

static inline category_t
runtime_ingress_memaccess_category(type_id src_type, bool after,
                                   const void *payload)
{
    switch (src_type) {
        case EVENT_MA_READ:
            return CAT_BEFORE_READ;
        case EVENT_MA_WRITE:
            return CAT_BEFORE_WRITE;
        case EVENT_MA_AREAD:
            return after ? CAT_AFTER_AREAD : CAT_BEFORE_AREAD;
        case EVENT_MA_AWRITE:
            return after ? CAT_AFTER_AWRITE : CAT_BEFORE_AWRITE;
        case EVENT_MA_RMW:
            return after ? CAT_AFTER_RMW : CAT_BEFORE_RMW;
        case EVENT_MA_XCHG:
            return after ? CAT_AFTER_XCHG : CAT_BEFORE_XCHG;
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            const struct ma_cmpxchg_event *ev = payload;
            if (!after) {
                return CAT_BEFORE_CMPXCHG;
            }
            return _context_memaccess_equal(ev->size, ev->old.u64,
                                            ev->val.u64) ?
                       CAT_AFTER_CMPXCHG_S :
                       CAT_AFTER_CMPXCHG_F;
        }
        case EVENT_MA_FENCE:
            return after ? CAT_AFTER_FENCE : CAT_BEFORE_FENCE;
        default:
            return CAT_NONE;
    }
}

static inline type_id
runtime_ingress_memaccess_type(type_id src_type, bool after, const void *payload)
{
    switch (src_type) {
        case EVENT_MA_READ:
            return EVENT_BEFORE_READ;
        case EVENT_MA_WRITE:
            return EVENT_BEFORE_WRITE;
        case EVENT_MA_AREAD:
            return after ? EVENT_AFTER_AREAD : EVENT_BEFORE_AREAD;
        case EVENT_MA_AWRITE:
            return after ? EVENT_AFTER_AWRITE : EVENT_BEFORE_AWRITE;
        case EVENT_MA_RMW:
            return after ? EVENT_AFTER_RMW : EVENT_BEFORE_RMW;
        case EVENT_MA_XCHG:
            return after ? EVENT_AFTER_XCHG : EVENT_BEFORE_XCHG;
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            const struct ma_cmpxchg_event *ev = payload;
            if (!after) {
                return EVENT_BEFORE_CMPXCHG;
            }
            return _context_memaccess_equal(ev->size, ev->old.u64,
                                            ev->val.u64) ?
                       EVENT_AFTER_CMPXCHG_S :
                       EVENT_AFTER_CMPXCHG_F;
        }
        case EVENT_MA_FENCE:
            return after ? EVENT_AFTER_FENCE : EVENT_BEFORE_FENCE;
        default:
            return 0;
    }
}

static inline context_t
runtime_ingress_memaccess_context(const context_t *origin,
                                  const capture_point *cp, bool after)
{
    context_t ctx = runtime_ingress_module_base(origin, cp);
    ctx.type      = runtime_ingress_memaccess_type(cp->src_type, after,
                                                   cp->payload);
    ctx.cat       = runtime_ingress_memaccess_category(cp->src_type, after,
                                                       cp->payload);
    ctx           = runtime_ingress_finalize_context(ctx, ctx.cat);
    ASSERT(ctx.type != 0);
    ASSERT(ctx.cat != CAT_NONE);
    switch (cp->src_type) {
        case EVENT_MA_READ: {
            const struct ma_read_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            break;
        }
        case EVENT_MA_WRITE: {
            const struct ma_write_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            break;
        }
        case EVENT_MA_AREAD: {
            const struct ma_aread_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            break;
        }
        case EVENT_MA_AWRITE: {
            const struct ma_awrite_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            ctx.args[2] = context_memaccess_sized_arg(ev->size, ev->val.u64);
            break;
        }
        case EVENT_MA_RMW: {
            const struct ma_rmw_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            ctx.args[2] = context_memaccess_sized_arg(ev->size, ev->val.u64);
            ctx.args[3] = arg(uint32_t, ev->op);
            break;
        }
        case EVENT_MA_XCHG: {
            const struct ma_xchg_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            ctx.args[2] = context_memaccess_sized_arg(ev->size, ev->val.u64);
            break;
        }
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            const struct ma_cmpxchg_event *ev = cp->payload;
            ctx.args[0] = arg_ptr(ev->addr);
            ctx.args[1] = arg(size_t, ev->size);
            ctx.args[2] = context_memaccess_sized_arg(ev->size, ev->cmp.u64);
            ctx.args[3] = context_memaccess_sized_arg(ev->size, ev->val.u64);
            break;
        }
        default:
            break;
    }
    return ctx;
}

static inline void
runtime_ingress_module_submit_memaccess(const context_t *origin,
                                        const capture_point *cp)
{
    context_t ctx = runtime_ingress_memaccess_context(origin, cp, false);
    runtime_ingress(&ctx);
}

static inline void
runtime_ingress_module_submit_memaccess_after(const context_t *origin,
                                              const capture_point *cp)
{
    context_t ctx = runtime_ingress_memaccess_context(origin, cp, true);
    runtime_ingress(&ctx);
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
