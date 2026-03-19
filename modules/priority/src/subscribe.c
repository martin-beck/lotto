#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include "category.h"
#include <lotto/engine/pubsub.h>
#include <lotto/base/context.h>
#include <lotto/modules/priority/events.h>
#include <lotto/priority.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/util/once.h>

static category_t CAT_PRIORITY;

typedef struct {
    int64_t priority;
} priority_event_t;

PS_ADVERTISE_TYPE(EVENT_PRIORITY)

static void
_lotto_priority(int64_t priority)
{
    once(CAT_PRIORITY = priority_category());
    context_t ctx       = *ctx(.self = self_md(), .func = __FUNCTION__);
    priority_event_t ev = {.priority = priority};
    capture_point cp    = {.src_type = EVENT_PRIORITY, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

static void
intercept_priority(int64_t priority)
{
    priority_event_t ev = {.priority = priority};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_PRIORITY, &ev, 0);
}

void
lotto_priority(int64_t priority)
{
    intercept_priority(priority);
}

void
lotto_priority_cond(bool cond, int64_t priority)
{
    if (!cond) {
        return;
    }
    lotto_priority(priority);
}

void
lotto_priority_task(task_id task, int64_t priority)
{
    if (get_task_id() != task) {
        return;
    }
    lotto_priority(priority);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_PRIORITY, {
    priority_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_priority(ev->priority);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;
    context_t ctx           = *origin;

    if (cp->src_type != EVENT_PRIORITY) {
        return PS_OK;
    }

    once(CAT_PRIORITY = priority_category());
    priority_event_t *ev = cp->payload;
    ctx.cat              = CAT_PRIORITY;
    ctx.args[0]          = arg(int64_t, ev->priority);
    runtime_ingress(&ctx);
    return PS_OK;
})
