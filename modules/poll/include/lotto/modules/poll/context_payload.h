/**
 * @file context_payload.h
 * @brief Accessors for poll-related context payloads.
 */
#ifndef LOTTO_MODULES_POLL_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_POLL_CONTEXT_PAYLOAD_H

#include "events.h"
#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/context_payload.h>

typedef enum context_poll_event {
    CONTEXT_POLL_NONE = 0,
    CONTEXT_POLL_WAIT,
} context_poll_event_t;

static inline context_poll_event_t
context_poll_event(const context_t *ctx)
{
    return context_has_event_type(ctx, EVENT_POLL) ? CONTEXT_POLL_WAIT :
                                                     CONTEXT_POLL_NONE;
}

static inline poll_args_t *
context_poll_args(const context_t *ctx)
{
    ASSERT(context_has_event_type(ctx, EVENT_POLL));
    ASSERT(context_has_capture_point(ctx));
    return ((poll_event_t *)ctx->cp->payload)->args;
}

#endif
