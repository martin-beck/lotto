/**
 * @file context_payload.h
 * @brief Accessors for task-velocity-related context payloads.
 */
#ifndef LOTTO_MODULES_TASK_VELOCITY_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_TASK_VELOCITY_CONTEXT_PAYLOAD_H

#include <stdint.h>

#include <lotto/base/context.h>
#include <lotto/modules/task_velocity/events.h>
#include <lotto/runtime/capture_point.h>

static inline uint64_t
context_task_velocity_probability(const context_t *ctx)
{
    if (context_has_capture_point(ctx) &&
        ctx->src_type == EVENT_TASK_VELOCITY) {
        return (uint64_t)
            ((task_velocity_event_t *)ctx->cp->payload)->probability;
    }
    return (uint64_t)ctx->args[0].value.u64;
}

#endif
