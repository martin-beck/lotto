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
#include <lotto/runtime/context_payload.h>

static inline bool
context_is_task_velocity_event(const context_t *ctx)
{
    return context_has_event_type(ctx, EVENT_TASK_VELOCITY);
}

static inline uint64_t
context_task_velocity_probability(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_TASK_VELOCITY));
    ASSERT(context_has_capture_point(ctx));
    return (uint64_t)((task_velocity_event_t *)ctx->cp->payload)->probability;
}

#endif
