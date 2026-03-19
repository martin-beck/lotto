#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

#include <dice/chains/capture.h>
#include <dice/self.h>
#include <dice/events/pthread.h>
#include <dice/events/thread.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/modules/yield/events.h>
#include <lotto/rsrc_deadlock.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/logger.h>

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_SCHED_YIELD, {
    context_t ctx    = *ctx(.self = md, .func = "sched_yield");
    capture_point cp = {.src_type = EVENT_USER_YIELD, .payload = NULL};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;

    if (cp->src_type != EVENT_USER_YIELD) {
        return PS_OK;
    }

    runtime_ingress_module_submit_auto(origin, cp);
    return PS_OK;
})
