/**
 * @file context_payload.h
 * @brief Accessors for region-preemption-related context payloads.
 */
#ifndef LOTTO_MODULES_REGION_PREEMPTION_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_REGION_PREEMPTION_CONTEXT_PAYLOAD_H

#include <stdbool.h>

#include <lotto/base/context.h>
#include <lotto/modules/region_preemption/events.h>
#include <lotto/runtime/capture_point.h>

static inline bool
context_region_preemption_in(const context_t *ctx)
{
    if (context_has_capture_point(ctx) &&
        ctx->src_type == EVENT_REGION_PREEMPTION) {
        return ((region_preemption_event_t *)ctx->cp->payload)->in_region;
    }
    return ctx->args[0].value.u64;
}

#endif
