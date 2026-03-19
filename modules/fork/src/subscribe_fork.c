#define LOGGER_BLOCK LOGGER_CUR_BLOCK

#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include "state.h"
#include <lotto/base/tidmap.h>
#include <lotto/engine/dispatcher.h>
#include <lotto/engine/pubsub.h>
#include <lotto/engine/statemgr.h>
#include <lotto/modules/fork/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/assert.h>
#include <lotto/sys/logger_block.h>
#include <lotto/sys/unistd.h>
#include <lotto/util/macros.h>
#include <lotto/util/once.h>

typedef struct {
    const char *func;
} fork_event_t;

PS_ADVERTISE_TYPE(EVENT_FORK_EXECVE)

pid_t
lotto_fork_execve(const char *pathname, char *const argv[], char *const envp[])
{
    if (!fork_execve_config()->enabled) {
        return -1;
    }

    pid_t ret = -1;
    fork_event_t ev = {.func = __FUNCTION__};

    PS_PUBLISH(INTERCEPT_BEFORE, EVENT_FORK_EXECVE, &ev, 0);

    ret = sys_fork();

    if (ret != 0) {
        PS_PUBLISH(INTERCEPT_AFTER, EVENT_FORK_EXECVE, &ev, 0);
    } else {
        ASSERT(sys_execve(pathname, argv, envp) == 0 &&
               "Error while calling execve");
        __builtin_unreachable();
    }

    return ret;
}

PS_SUBSCRIBE(CAPTURE_BEFORE, EVENT_FORK_EXECVE, {
    fork_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_FORK_EXECVE, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CAPTURE_AFTER, EVENT_FORK_EXECVE, {
    fork_event_t *ev = EVENT_PAYLOAD(event);
    context_t ctx    = *ctx(.self = self_md(), .func = ev->func);
    capture_point cp = {.src_type = EVENT_FORK_EXECVE, .payload = ev};
    PS_PUBLISH(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, &cp,
               (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_FORK_EXECVE) {
        return PS_OK;
    }

    ctx          = *origin;
    ctx.type     = EVENT_MODULE_INTERCEPT;
    ctx.src_type = cp->src_type;
    ctx.cat      = CAT_CALL;
    (void)runtime_ingress_before(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx;

    if (cp->src_type != EVENT_FORK_EXECVE) {
        return PS_OK;
    }

    ctx          = *origin;
    ctx.type     = EVENT_MODULE_INTERCEPT;
    ctx.src_type = cp->src_type;
    ctx.cat      = CAT_CALL;
    runtime_ingress_after(&ctx);
    return PS_OK;
})
