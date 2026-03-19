#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/self.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/engine/pubsub.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/modules/task_velocity/events.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/assert.h>
#include <lotto/velocity.h>

PS_ADVERTISE_TYPE(EVENT_TASK_VELOCITY)

static void
_lotto_task_velocity(int64_t probability)
{
    context_t ctx            = *ctx(.self = self_md(), .func = __FUNCTION__);
    task_velocity_event_t ev = {.probability = probability};
    capture_point cp         = {.src_type = EVENT_TASK_VELOCITY,
                                .payload  = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

static void
intercept_task_velocity(int64_t probability)
{
    task_velocity_event_t ev = {.probability = probability};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_TASK_VELOCITY, &ev, 0);
}

void
lotto_task_velocity(int64_t probability)
{
    ASSERT(probability >= LOTTO_TASK_VELOCITY_MIN &&
           probability <= LOTTO_TASK_VELOCITY_MAX);

    intercept_task_velocity(probability);
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_TASK_VELOCITY, {
    task_velocity_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_task_velocity(ev->probability);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_t *origin = (const context_t *)md;
    capture_point *cp       = (capture_point *)event;

    if (cp->src_type != EVENT_TASK_VELOCITY) {
        return PS_OK;
    }

    runtime_ingress_module_submit(origin, cp, CAT_TASK_VELOCITY);
    return PS_OK;
})
