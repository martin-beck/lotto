/**
 * @file ingress_capture.h
 * @brief Narrow runtime ingress capture declarations.
 */
#ifndef LOTTO_INGRESS_CAPTURE_H
#define LOTTO_INGRESS_CAPTURE_H

#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_origin.h>
#include <lotto/runtime/context_payload.h>

typedef struct ingress_capture {
    context_origin origin;
    type_id type;
    type_id src_type;
    const capture_point *cp;
    context_phase_t phase;
    category_t fallback_cat;
} ingress_capture;

static inline context_t
runtime_context_from_origin(const context_origin *origin)
{
    ASSERT(origin != NULL);
    return (context_t){
        ._    = origin->_,
        .self = origin->self,
        .id   = origin->id,
        .vid  = origin->vid,
        .pc   = origin->pc,
#if defined(QLOTTO_ENABLED)
        .icount = origin->icount,
        .pstate = origin->pstate,
#endif
        .func      = origin->func,
        .func_addr = origin->func_addr,
        .phase     = CONTEXT_PHASE_EVENT,
    };
}

static inline ingress_capture
runtime_ingress_capture_base_phase(const context_origin *origin, type_id type,
                                   const capture_point *cp,
                                   context_phase_t phase,
                                   category_t fallback_cat)
{
    ASSERT(origin != NULL);
    return (ingress_capture){
        .origin       = *origin,
        .type         = type,
        .src_type     = cp != NULL ? cp->src_type : type,
        .cp           = cp,
        .phase        = phase,
        .fallback_cat = fallback_cat,
    };
}

static inline ingress_capture
runtime_ingress_capture_base(const context_origin *origin, type_id type,
                             const capture_point *cp, category_t fallback_cat)
{
    return runtime_ingress_capture_base_phase(origin, type, cp,
                                              CONTEXT_PHASE_EVENT,
                                              fallback_cat);
}

static inline ingress_capture
runtime_ingress_capture_synthetic(const char *func, type_id type)
{
    return runtime_ingress_capture_base(
        &(context_origin){.id = NO_TASK, .vid = NO_TASK, .func = func}, type,
        NULL, CAT_NONE);
}

static inline context_t
runtime_context_from_ingress_capture(const ingress_capture *capture)
{
    ASSERT(capture != NULL);
    context_t ctx = runtime_context_from_origin(&capture->origin);
    ctx.cp        = (capture_point *)capture->cp;
    ctx.type      = capture->type;
    ctx.src_type  = capture->src_type;
    ctx.phase     = capture->phase;
    return context_finalize_category(ctx, capture->fallback_cat);
}

static inline context_t
runtime_context_synthetic(const char *func, type_id type)
{
    ingress_capture capture = runtime_ingress_capture_synthetic(func, type);
    return runtime_context_from_ingress_capture(&capture);
}

#endif
