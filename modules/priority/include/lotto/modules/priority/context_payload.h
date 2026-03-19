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

static inline int64_t
context_priority_value(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_PRIORITY) {
        return ((priority_event_t *)ctx->cp->payload)->priority;
    }
    return (int64_t)ctx->args[0].value.u64;
}

#endif
