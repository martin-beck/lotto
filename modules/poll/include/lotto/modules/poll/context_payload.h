/**
 * @file context_payload.h
 * @brief Accessors for poll-related context payloads.
 */
#ifndef LOTTO_MODULES_POLL_CONTEXT_PAYLOAD_H
#define LOTTO_MODULES_POLL_CONTEXT_PAYLOAD_H

#include "events.h"
#include <lotto/base/context.h>
#include <lotto/runtime/capture_point.h>

static inline poll_args_t *
context_poll_args(const context_t *ctx)
{
    if (context_has_capture_point(ctx) && ctx->src_type == EVENT_POLL) {
        return ((poll_event_t *)ctx->cp->payload)->args;
    }
    return (poll_args_t *)ctx->args[0].value.ptr;
}

#endif
