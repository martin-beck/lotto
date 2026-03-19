/**
 * @file context_payload.h
 * @brief Accessors for priority-related context payloads.
 */
#ifndef LOTTO_MODULES_PRIORITY_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_PRIORITY_CONTEXT_PAYLOAD_H

#include <stdint.h>

#include <lotto/base/context.h>
#include <lotto/modules/priority/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>

static inline bool
context_is_priority_event(const context_t *ctx)
{
    return context_has_event_type(ctx, EVENT_PRIORITY);
}

static inline int64_t
context_priority_value(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_PRIORITY));
    ASSERT(context_has_capture_point(ctx));
    return ((priority_event_t *)ctx->cp->payload)->priority;
}

#endif
