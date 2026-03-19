/**
 * @file memaccess_payload.h
 * @brief Accessors for generic memaccess-related context payloads.
 */
#ifndef LOTTO_RUNTIME_MEMACCESS_PAYLOAD_H
#define LOTTO_RUNTIME_MEMACCESS_PAYLOAD_H

#include <stdint.h>

#include <dice/events/memaccess.h>
#include <lotto/base/arg.h>
#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/assert.h>

typedef enum context_memaccess_event {
    CONTEXT_MA_NONE = 0,
    CONTEXT_MA_BEFORE_READ,
    CONTEXT_MA_BEFORE_WRITE,
    CONTEXT_MA_BEFORE_AREAD,
    CONTEXT_MA_BEFORE_AWRITE,
    CONTEXT_MA_BEFORE_RMW,
    CONTEXT_MA_BEFORE_XCHG,
    CONTEXT_MA_BEFORE_CMPXCHG,
    CONTEXT_MA_BEFORE_FENCE,
    CONTEXT_MA_AFTER_AREAD,
    CONTEXT_MA_AFTER_AWRITE,
    CONTEXT_MA_AFTER_RMW,
    CONTEXT_MA_AFTER_XCHG,
    CONTEXT_MA_AFTER_CMPXCHG_S,
    CONTEXT_MA_AFTER_CMPXCHG_F,
    CONTEXT_MA_AFTER_FENCE,
} context_memaccess_event_t;

static inline category_t
context_memaccess_category_from_event(context_memaccess_event_t event)
{
    switch (event) {
        case CONTEXT_MA_BEFORE_READ:
            return CAT_BEFORE_READ;
        case CONTEXT_MA_BEFORE_WRITE:
            return CAT_BEFORE_WRITE;
        case CONTEXT_MA_BEFORE_AREAD:
            return CAT_BEFORE_AREAD;
        case CONTEXT_MA_BEFORE_AWRITE:
            return CAT_BEFORE_AWRITE;
        case CONTEXT_MA_BEFORE_RMW:
            return CAT_BEFORE_RMW;
        case CONTEXT_MA_BEFORE_XCHG:
            return CAT_BEFORE_XCHG;
        case CONTEXT_MA_BEFORE_CMPXCHG:
            return CAT_BEFORE_CMPXCHG;
        case CONTEXT_MA_BEFORE_FENCE:
            return CAT_BEFORE_FENCE;
        case CONTEXT_MA_AFTER_AREAD:
            return CAT_AFTER_AREAD;
        case CONTEXT_MA_AFTER_AWRITE:
            return CAT_AFTER_AWRITE;
        case CONTEXT_MA_AFTER_RMW:
            return CAT_AFTER_RMW;
        case CONTEXT_MA_AFTER_XCHG:
            return CAT_AFTER_XCHG;
        case CONTEXT_MA_AFTER_CMPXCHG_S:
            return CAT_AFTER_CMPXCHG_S;
        case CONTEXT_MA_AFTER_CMPXCHG_F:
            return CAT_AFTER_CMPXCHG_F;
        case CONTEXT_MA_AFTER_FENCE:
            return CAT_AFTER_FENCE;
        default:
            return CAT_NONE;
    }
}

static inline type_id
context_memaccess_type_from_event(context_memaccess_event_t event)
{
    switch (event) {
        case CONTEXT_MA_BEFORE_READ:
            return EVENT_BEFORE_READ;
        case CONTEXT_MA_BEFORE_WRITE:
            return EVENT_BEFORE_WRITE;
        case CONTEXT_MA_BEFORE_AREAD:
            return EVENT_BEFORE_AREAD;
        case CONTEXT_MA_BEFORE_AWRITE:
            return EVENT_BEFORE_AWRITE;
        case CONTEXT_MA_BEFORE_RMW:
            return EVENT_BEFORE_RMW;
        case CONTEXT_MA_BEFORE_XCHG:
            return EVENT_BEFORE_XCHG;
        case CONTEXT_MA_BEFORE_CMPXCHG:
            return EVENT_BEFORE_CMPXCHG;
        case CONTEXT_MA_BEFORE_FENCE:
            return EVENT_BEFORE_FENCE;
        case CONTEXT_MA_AFTER_AREAD:
            return EVENT_AFTER_AREAD;
        case CONTEXT_MA_AFTER_AWRITE:
            return EVENT_AFTER_AWRITE;
        case CONTEXT_MA_AFTER_RMW:
            return EVENT_AFTER_RMW;
        case CONTEXT_MA_AFTER_XCHG:
            return EVENT_AFTER_XCHG;
        case CONTEXT_MA_AFTER_CMPXCHG_S:
            return EVENT_AFTER_CMPXCHG_S;
        case CONTEXT_MA_AFTER_CMPXCHG_F:
            return EVENT_AFTER_CMPXCHG_F;
        case CONTEXT_MA_AFTER_FENCE:
            return EVENT_AFTER_FENCE;
        default:
            return 0;
    }
}

static inline bool
_context_memaccess_equal(size_t size, uint64_t lhs, uint64_t rhs)
{
    switch (size) {
        case 1:
            return (uint8_t)lhs == (uint8_t)rhs;
        case 2:
            return (uint16_t)lhs == (uint16_t)rhs;
        case 4:
            return (uint32_t)lhs == (uint32_t)rhs;
        case 8:
            return lhs == rhs;
        default:
            ASSERT(0);
            return false;
    }
}

static inline context_memaccess_event_t
context_memaccess_event_from_source(type_id src_type, const void *payload,
                                    context_phase_t phase,
                                    category_t phase_cat)
{
    switch (src_type) {
        case EVENT_MA_READ:
            return CONTEXT_MA_BEFORE_READ;
        case EVENT_MA_WRITE:
            return CONTEXT_MA_BEFORE_WRITE;
        case EVENT_MA_AREAD:
            return phase == CONTEXT_PHASE_AFTER ? CONTEXT_MA_AFTER_AREAD :
                                                  CONTEXT_MA_BEFORE_AREAD;
        case EVENT_MA_AWRITE:
            return phase == CONTEXT_PHASE_AFTER ? CONTEXT_MA_AFTER_AWRITE :
                                                  CONTEXT_MA_BEFORE_AWRITE;
        case EVENT_MA_RMW:
            return phase == CONTEXT_PHASE_AFTER ? CONTEXT_MA_AFTER_RMW :
                                                  CONTEXT_MA_BEFORE_RMW;
        case EVENT_MA_XCHG:
            return phase == CONTEXT_PHASE_AFTER ? CONTEXT_MA_AFTER_XCHG :
                                                  CONTEXT_MA_BEFORE_XCHG;
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK: {
            const struct ma_cmpxchg_event *ev = payload;
            if (phase != CONTEXT_PHASE_AFTER) {
                return CONTEXT_MA_BEFORE_CMPXCHG;
            }
            return _context_memaccess_equal(ev->size, ev->old.u64,
                                            ev->val.u64) ?
                       CONTEXT_MA_AFTER_CMPXCHG_S :
                       CONTEXT_MA_AFTER_CMPXCHG_F;
        }
        case EVENT_MA_FENCE:
            return phase == CONTEXT_PHASE_AFTER ? CONTEXT_MA_AFTER_FENCE :
                                                  CONTEXT_MA_BEFORE_FENCE;
        default:
            return CONTEXT_MA_NONE;
    }
}

static inline category_t
context_memaccess_phase_category(type_id type)
{
    switch (type) {
        case EVENT_BEFORE_READ:
            return CAT_BEFORE_READ;
        case EVENT_BEFORE_WRITE:
            return CAT_BEFORE_WRITE;
        case EVENT_BEFORE_AREAD:
            return CAT_BEFORE_AREAD;
        case EVENT_BEFORE_AWRITE:
            return CAT_BEFORE_AWRITE;
        case EVENT_BEFORE_RMW:
            return CAT_BEFORE_RMW;
        case EVENT_BEFORE_XCHG:
            return CAT_BEFORE_XCHG;
        case EVENT_BEFORE_CMPXCHG:
            return CAT_BEFORE_CMPXCHG;
        case EVENT_BEFORE_FENCE:
            return CAT_BEFORE_FENCE;
        case EVENT_AFTER_AREAD:
            return CAT_AFTER_AREAD;
        case EVENT_AFTER_AWRITE:
            return CAT_AFTER_AWRITE;
        case EVENT_AFTER_RMW:
            return CAT_AFTER_RMW;
        case EVENT_AFTER_XCHG:
            return CAT_AFTER_XCHG;
        case EVENT_AFTER_CMPXCHG_S:
        case EVENT_AFTER_CMPXCHG_F:
            return CAT_AFTER_CMPXCHG_S;
        case EVENT_AFTER_FENCE:
            return CAT_AFTER_FENCE;
        default:
            return CAT_NONE;
    }
}

static inline context_memaccess_event_t
context_memaccess_event(const context_t *ctx)
{
    if (context_has_type(ctx)) {
        switch (ctx->type) {
            case EVENT_BEFORE_READ:
                return CONTEXT_MA_BEFORE_READ;
            case EVENT_BEFORE_WRITE:
                return CONTEXT_MA_BEFORE_WRITE;
            case EVENT_BEFORE_AREAD:
                return CONTEXT_MA_BEFORE_AREAD;
            case EVENT_BEFORE_AWRITE:
                return CONTEXT_MA_BEFORE_AWRITE;
            case EVENT_BEFORE_RMW:
                return CONTEXT_MA_BEFORE_RMW;
            case EVENT_BEFORE_XCHG:
                return CONTEXT_MA_BEFORE_XCHG;
            case EVENT_BEFORE_CMPXCHG:
                return CONTEXT_MA_BEFORE_CMPXCHG;
            case EVENT_BEFORE_FENCE:
                return CONTEXT_MA_BEFORE_FENCE;
            case EVENT_AFTER_AREAD:
                return CONTEXT_MA_AFTER_AREAD;
            case EVENT_AFTER_AWRITE:
                return CONTEXT_MA_AFTER_AWRITE;
            case EVENT_AFTER_RMW:
                return CONTEXT_MA_AFTER_RMW;
            case EVENT_AFTER_XCHG:
                return CONTEXT_MA_AFTER_XCHG;
            case EVENT_AFTER_CMPXCHG_S:
                return CONTEXT_MA_AFTER_CMPXCHG_S;
            case EVENT_AFTER_CMPXCHG_F:
                return CONTEXT_MA_AFTER_CMPXCHG_F;
            case EVENT_AFTER_FENCE:
                return CONTEXT_MA_AFTER_FENCE;
            default:
                break;
        }
    }
    if (context_has_capture_point(ctx)) {
        context_phase_t phase = ctx->phase;
        category_t phase_cat = context_memaccess_phase_category(ctx->type);
        if (phase_cat == CAT_NONE) {
            phase_cat = context_compat_category(ctx);
        }
        if (phase == CONTEXT_PHASE_EVENT) {
            phase = phase_cat == CAT_NONE || phase_cat == CAT_BEFORE_READ ||
                            phase_cat == CAT_BEFORE_WRITE ||
                            phase_cat == CAT_BEFORE_AREAD ||
                            phase_cat == CAT_BEFORE_AWRITE ||
                            phase_cat == CAT_BEFORE_RMW ||
                            phase_cat == CAT_BEFORE_XCHG ||
                            phase_cat == CAT_BEFORE_CMPXCHG ||
                            phase_cat == CAT_BEFORE_FENCE ?
                        CONTEXT_PHASE_BEFORE :
                        CONTEXT_PHASE_AFTER;
        }
        return context_memaccess_event_from_source(context_event_type(ctx),
                                                   ctx->cp->payload, phase,
                                                   phase_cat);
    }
    switch (context_compat_category(ctx)) {
        case CAT_BEFORE_READ:
            return CONTEXT_MA_BEFORE_READ;
        case CAT_BEFORE_WRITE:
            return CONTEXT_MA_BEFORE_WRITE;
        case CAT_BEFORE_AREAD:
            return CONTEXT_MA_BEFORE_AREAD;
        case CAT_BEFORE_AWRITE:
            return CONTEXT_MA_BEFORE_AWRITE;
        case CAT_BEFORE_RMW:
            return CONTEXT_MA_BEFORE_RMW;
        case CAT_BEFORE_XCHG:
            return CONTEXT_MA_BEFORE_XCHG;
        case CAT_BEFORE_CMPXCHG:
            return CONTEXT_MA_BEFORE_CMPXCHG;
        case CAT_BEFORE_FENCE:
            return CONTEXT_MA_BEFORE_FENCE;
        case CAT_AFTER_AREAD:
            return CONTEXT_MA_AFTER_AREAD;
        case CAT_AFTER_AWRITE:
            return CONTEXT_MA_AFTER_AWRITE;
        case CAT_AFTER_RMW:
            return CONTEXT_MA_AFTER_RMW;
        case CAT_AFTER_XCHG:
            return CONTEXT_MA_AFTER_XCHG;
        case CAT_AFTER_CMPXCHG_S:
            return CONTEXT_MA_AFTER_CMPXCHG_S;
        case CAT_AFTER_CMPXCHG_F:
            return CONTEXT_MA_AFTER_CMPXCHG_F;
        case CAT_AFTER_FENCE:
            return CONTEXT_MA_AFTER_FENCE;
        default:
            return CONTEXT_MA_NONE;
    }
}

static inline category_t
context_memaccess_category(const context_t *ctx)
{
    return context_memaccess_category_from_event(context_memaccess_event(ctx));
}

static inline arg_t
context_memaccess_sized_arg(size_t size, uint64_t value)
{
    switch (size) {
        case 1:
            return (arg_t){.value.u8 = (uint8_t)value, .width = ARG_U8};
        case 2:
            return (arg_t){.value.u16 = (uint16_t)value, .width = ARG_U16};
        case 4:
            return (arg_t){.value.u32 = (uint32_t)value, .width = ARG_U32};
        case 8:
            return (arg_t){.value.u64 = value, .width = ARG_U64};
        default:
            ASSERT(0);
            return (arg_t){0};
    }
}

static inline uintptr_t
context_memaccess_addr(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (ctx->src_type) {
        case EVENT_MA_READ:
            return (uintptr_t)((struct ma_read_event *)ctx->cp->payload)->addr;
        case EVENT_MA_WRITE:
            return (uintptr_t)((struct ma_write_event *)ctx->cp->payload)->addr;
        case EVENT_MA_AREAD:
            return (uintptr_t)((struct ma_aread_event *)ctx->cp->payload)->addr;
        case EVENT_MA_AWRITE:
            return (uintptr_t)((struct ma_awrite_event *)ctx->cp->payload)
                ->addr;
        case EVENT_MA_RMW:
            return (uintptr_t)((struct ma_rmw_event *)ctx->cp->payload)->addr;
        case EVENT_MA_XCHG:
            return (uintptr_t)((struct ma_xchg_event *)ctx->cp->payload)->addr;
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
            return (uintptr_t)((struct ma_cmpxchg_event *)ctx->cp->payload)
                ->addr;
        default:
            ASSERT(0);
            return 0;
    }
}

static inline size_t
context_memaccess_size(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (ctx->src_type) {
        case EVENT_MA_READ:
            return ((struct ma_read_event *)ctx->cp->payload)->size;
        case EVENT_MA_WRITE:
            return ((struct ma_write_event *)ctx->cp->payload)->size;
        case EVENT_MA_AREAD:
            return ((struct ma_aread_event *)ctx->cp->payload)->size;
        case EVENT_MA_AWRITE:
            return ((struct ma_awrite_event *)ctx->cp->payload)->size;
        case EVENT_MA_RMW:
            return ((struct ma_rmw_event *)ctx->cp->payload)->size;
        case EVENT_MA_XCHG:
            return ((struct ma_xchg_event *)ctx->cp->payload)->size;
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
            return ((struct ma_cmpxchg_event *)ctx->cp->payload)->size;
        default:
            ASSERT(0);
            return 0;
    }
}

static inline arg_t
context_memaccess_value(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (ctx->src_type) {
        case EVENT_MA_AWRITE:
            return context_memaccess_sized_arg(
                ((struct ma_awrite_event *)ctx->cp->payload)->size,
                ((struct ma_awrite_event *)ctx->cp->payload)->val.u64);
        case EVENT_MA_RMW:
            return context_memaccess_sized_arg(
                ((struct ma_rmw_event *)ctx->cp->payload)->size,
                ((struct ma_rmw_event *)ctx->cp->payload)->val.u64);
        case EVENT_MA_XCHG:
            return context_memaccess_sized_arg(
                ((struct ma_xchg_event *)ctx->cp->payload)->size,
                ((struct ma_xchg_event *)ctx->cp->payload)->val.u64);
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
            return context_memaccess_sized_arg(
                ((struct ma_cmpxchg_event *)ctx->cp->payload)->size,
                ((struct ma_cmpxchg_event *)ctx->cp->payload)->val.u64);
        default:
            return (arg_t){0};
    }
}

static inline arg_t
context_memaccess_cmp(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    switch (ctx->src_type) {
        case EVENT_MA_CMPXCHG:
        case EVENT_MA_CMPXCHG_WEAK:
            return context_memaccess_sized_arg(
                ((struct ma_cmpxchg_event *)ctx->cp->payload)->size,
                ((struct ma_cmpxchg_event *)ctx->cp->payload)->cmp.u64);
        default:
            return (arg_t){0};
    }
}

static inline uint32_t
context_memaccess_rmw_op(const context_t *ctx)
{
    ASSERT(context_has_capture_point(ctx));
    ASSERT(ctx->src_type == EVENT_MA_RMW);
    return (uint32_t)((struct ma_rmw_event *)ctx->cp->payload)->op;
}

#endif
