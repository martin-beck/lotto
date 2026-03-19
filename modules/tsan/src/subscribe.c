#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/events/pthread.h>
#include <dice/events/stacktrace.h>
#include <dice/events/thread.h>
#include <dice/interpose.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/rsrc_deadlock.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/logger.h>

#define EV_PC ((uintptr_t)ev->pc)
#define PUBLISH_TSAN_INGRESS(CHAIN, SRC_TYPE, PAYLOAD, FUNC)                   \
    do {                                                                       \
        context_t ctx = *ctx_pc(.self = md, .pc = EV_PC, .func = (FUNC));      \
        capture_point cp = {.src_type = (SRC_TYPE), .payload = (PAYLOAD)};     \
        PS_PUBLISH((CHAIN), EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);  \
    } while (0)

// -----------------------------------------------------------------------------
// memory accesses
//
// EVENT_MA_READ                 30       ./include/dice/events/memaccess.h
// EVENT_MA_WRITE                31       ./include/dice/events/memaccess.h
// EVENT_MA_AREAD                32       ./include/dice/events/memaccess.h
// EVENT_MA_AWRITE               33       ./include/dice/events/memaccess.h
// EVENT_MA_RMW                  34       ./include/dice/events/memaccess.h
// EVENT_MA_XCHG                 35       ./include/dice/events/memaccess.h
// EVENT_MA_CMPXCHG              36       ./include/dice/events/memaccess.h
// EVENT_MA_CMPXCHG_WEAK         37       ./include/dice/events/memaccess.h
// EVENT_MA_FENCE                38       ./include/dice/events/memaccess.h
// -----------------------------------------------------------------------------
#include <dice/events/memaccess.h>

#define sized_arg(s, uv)                                                       \
    ({                                                                         \
        arg_t _arg;                                                            \
        switch (s) {                                                           \
            case 1:                                                            \
                _arg = (arg_t){.value.u8 = uv.u8, .width = ARG_U8};            \
                break;                                                         \
            case 2:                                                            \
                _arg = (arg_t){.value.u16 = uv.u16, .width = ARG_U16};         \
                break;                                                         \
            case 4:                                                            \
                _arg = (arg_t){.value.u32 = uv.u32, .width = ARG_U32};         \
                break;                                                         \
            case 8:                                                            \
                _arg = (arg_t){.value.u64 = uv.u64, .width = ARG_U64};         \
                break;                                                         \
            default:                                                           \
                ASSERT(0);                                                     \
                break;                                                         \
        };                                                                     \
        _arg;                                                                  \
    })

#define sized_eq(s, uv1, uv2)                                                  \
    ({                                                                         \
        bool _result;                                                          \
        switch (s) {                                                           \
            case 1:                                                            \
                _result = uv1.u8 == uv2.u8;                                    \
                break;                                                         \
            case 2:                                                            \
                _result = uv1.u16 == uv2.u16;                                  \
                break;                                                         \
            case 4:                                                            \
                _result = uv1.u32 == uv2.u32;                                  \
                break;                                                         \
            case 8:                                                            \
                _result = uv1.u64 == uv2.u64;                                  \
                break;                                                         \
            default:                                                           \
                ASSERT(0);                                                     \
                break;                                                         \
        };                                                                     \
        _result;                                                               \
    })

static void
ingress_addr_size(const context_t *origin, category_t cat, const void *addr,
                  size_t size)
{
    context_t ctx = *origin;
    ctx.cat       = cat;
    ctx.args[0]   = arg_ptr(addr);
    ctx.args[1]   = arg(size_t, size);
    runtime_ingress(&ctx);
}

static void
ingress_addr_size_val(const context_t *origin, category_t cat, const void *addr,
                      size_t size, arg_t value)
{
    context_t ctx = *origin;
    ctx.cat       = cat;
    ctx.args[0]   = arg_ptr(addr);
    ctx.args[1]   = arg(size_t, size);
    ctx.args[2]   = value;
    runtime_ingress(&ctx);
}

static void
ingress_addr_size_val_op(const context_t *origin, category_t cat,
                         const void *addr, size_t size, arg_t value,
                         uint32_t op)
{
    context_t ctx = *origin;
    ctx.cat       = cat;
    ctx.args[0]   = arg_ptr(addr);
    ctx.args[1]   = arg(size_t, size);
    ctx.args[2]   = value;
    ctx.args[3]   = arg(uint32_t, op);
    runtime_ingress(&ctx);
}

static void
ingress_cmpxchg(const context_t *origin, category_t cat, const void *addr,
                size_t size, arg_t cmp, arg_t value)
{
    context_t ctx = *origin;
    ctx.cat       = cat;
    ctx.args[0]   = arg_ptr(addr);
    ctx.args[1]   = arg(size_t, size);
    ctx.args[2]   = cmp;
    ctx.args[3]   = value;
    runtime_ingress(&ctx);
}

static void
ingress_fence(const context_t *origin, category_t cat)
{
    context_t ctx = *origin;
    ctx.cat       = cat;
    runtime_ingress(&ctx);
}

static void
ingress_stacktrace_enter(const context_t *origin, const void *caller)
{
    context_t ctx = *origin;
    ctx.cat       = CAT_FUNC_ENTRY;
    ctx.args[0]   = arg_ptr(caller);
    runtime_ingress(&ctx);
}

static void
ingress_stacktrace_exit(const context_t *origin)
{
    context_t ctx = *origin;
    ctx.cat       = CAT_FUNC_EXIT;
    runtime_ingress(&ctx);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MA_READ, {
    struct ma_read_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_MA_READ, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_MA_WRITE, {
    struct ma_write_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_MA_WRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_AREAD, {
    struct ma_aread_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_AREAD, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_AREAD, {
    struct ma_aread_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_AREAD, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_AWRITE, {
    struct ma_awrite_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_AWRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_AWRITE, {
    struct ma_awrite_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_AWRITE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_RMW, {
    struct ma_rmw_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_RMW, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_RMW, {
    struct ma_rmw_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_RMW, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_XCHG, {
    struct ma_xchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_XCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_XCHG, {
    struct ma_xchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_XCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_CMPXCHG, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_CMPXCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_CMPXCHG, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_CMPXCHG, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_CMPXCHG_WEAK, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_CMPXCHG_WEAK, ev,
                         ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_CMPXCHG_WEAK, {
    struct ma_cmpxchg_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_CMPXCHG_WEAK, ev,
                         ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_MA_FENCE, {
    struct ma_fence_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_BEFORE, EVENT_MA_FENCE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_MA_FENCE, {
    struct ma_fence_event *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS_AFTER, EVENT_MA_FENCE, ev, ev->func);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_STACKTRACE_ENTER, {
    stacktrace_event_t *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_STACKTRACE_ENTER, ev,
                         "func_entry");
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_STACKTRACE_EXIT, {
    stacktrace_event_t *ev = EVENT_PAYLOAD(event);
    PUBLISH_TSAN_INGRESS(CHAIN_INGRESS, EVENT_STACKTRACE_EXIT, ev, "func_exit");
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = runtime_ingress_module_base(origin, cp);

    switch (cp->src_type) {
        case EVENT_MA_READ: {
            struct ma_read_event *ev = cp->payload;
            ingress_addr_size(&ctx, CAT_BEFORE_READ, ev->addr, ev->size);
            return PS_OK;
        }
        case EVENT_MA_WRITE: {
            struct ma_write_event *ev = cp->payload;
            ingress_addr_size(&ctx, CAT_BEFORE_WRITE, ev->addr, ev->size);
            return PS_OK;
        }
        case EVENT_STACKTRACE_ENTER: {
            stacktrace_event_t *ev = cp->payload;
            ingress_stacktrace_enter(&ctx, ev->caller);
            return PS_OK;
        }
        case EVENT_STACKTRACE_EXIT:
            ingress_stacktrace_exit(&ctx);
            return PS_OK;
        default:
            return PS_OK;
    }
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = runtime_ingress_module_base(origin, cp);

    switch (cp->src_type) {
        case EVENT_MA_AREAD: {
            struct ma_aread_event *ev = cp->payload;
            ingress_addr_size(&ctx, CAT_BEFORE_AREAD, ev->addr, ev->size);
            return PS_OK;
        }
        case EVENT_MA_AWRITE: {
            struct ma_awrite_event *ev = cp->payload;
            ingress_addr_size_val(&ctx, CAT_BEFORE_AWRITE, ev->addr, ev->size,
                                  sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_RMW: {
            struct ma_rmw_event *ev = cp->payload;
            ingress_addr_size_val_op(&ctx, CAT_BEFORE_RMW, ev->addr,
                                     ev->size, sized_arg(ev->size, ev->val),
                                     ev->op);
            return PS_OK;
        }
        case EVENT_MA_XCHG: {
            struct ma_xchg_event *ev = cp->payload;
            ingress_addr_size_val(&ctx, CAT_BEFORE_XCHG, ev->addr, ev->size,
                                  sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            struct ma_cmpxchg_event *ev = cp->payload;
            ingress_cmpxchg(&ctx, CAT_BEFORE_CMPXCHG, ev->addr, ev->size,
                            sized_arg(ev->size, ev->cmp),
                            sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_FENCE:
            ingress_fence(&ctx, CAT_BEFORE_FENCE);
            return PS_OK;
        default:
            return PS_OK;
    }
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = runtime_ingress_module_base(origin, cp);

    switch (cp->src_type) {
        case EVENT_MA_AREAD: {
            struct ma_aread_event *ev = cp->payload;
            ingress_addr_size(&ctx, CAT_AFTER_AREAD, ev->addr, ev->size);
            return PS_OK;
        }
        case EVENT_MA_AWRITE: {
            struct ma_awrite_event *ev = cp->payload;
            ingress_addr_size_val(&ctx, CAT_AFTER_AWRITE, ev->addr, ev->size,
                                  sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_RMW: {
            struct ma_rmw_event *ev = cp->payload;
            ingress_addr_size_val_op(&ctx, CAT_AFTER_RMW, ev->addr, ev->size,
                                     sized_arg(ev->size, ev->val), ev->op);
            return PS_OK;
        }
        case EVENT_MA_XCHG: {
            struct ma_xchg_event *ev = cp->payload;
            ingress_addr_size_val(&ctx, CAT_AFTER_XCHG, ev->addr, ev->size,
                                  sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            struct ma_cmpxchg_event *ev = cp->payload;
            ingress_cmpxchg(&ctx,
                            sized_eq(ev->size, ev->old, ev->val) ?
                                CAT_AFTER_CMPXCHG_S :
                                CAT_AFTER_CMPXCHG_F,
                            ev->addr, ev->size, sized_arg(ev->size, ev->cmp),
                            sized_arg(ev->size, ev->val));
            return PS_OK;
        }
        case EVENT_MA_FENCE:
            ingress_fence(&ctx, CAT_AFTER_FENCE);
            return PS_OK;
        default:
            return PS_OK;
    }
})
