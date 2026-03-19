#include "state.h"
#include <dice/chains/capture.h>
#include <dice/chains/intercept.h>
#include <dice/module.h>
#include <dice/pubsub.h>
#include <dice/self.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/modules/region_preemption/events.h>
#include <lotto/region_preemption.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

PS_ADVERTISE_TYPE(EVENT_REGION_PREEMPTION)

static void
_lotto_region_preemption_switch(bool in_region)
{
    context_origin ctx = *ctx_origin(.self = self_md(), .func = __FUNCTION__);
    region_preemption_event_t ev = {.in_region = in_region};
    capture_point cp = {.src_type = EVENT_REGION_PREEMPTION, .payload = &ev};
    PS_PUBLISH(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, &cp, (metadata_t *)&ctx);
}

static void
intercept_region_preemption_switch(bool in_region)
{
    region_preemption_event_t ev = {.in_region = in_region};
    PS_PUBLISH(INTERCEPT_EVENT, EVENT_REGION_PREEMPTION, &ev, 0);
}

void
lotto_region_preemption_switch(bool in_region)
{
    intercept_region_preemption_switch(in_region);
}

void
lotto_region_preemption_switch_cond(bool cond, bool in_region)
{
    if (!cond) {
        return;
    }
    lotto_region_preemption_switch(in_region);
}

void
lotto_region_preemption_switch_task(task_id task, bool in_region)
{
    if (get_task_id() != task) {
        return;
    }
    lotto_region_preemption_switch(in_region);
}

void
_lotto_region_atomic_enter()
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch(true);
    }
}
void
_lotto_region_atomic_leave()
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch(false);
    }
}
void
_lotto_region_atomic_enter_cond(bool cond)
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch_cond(cond, true);
    }
}
void
_lotto_region_atomic_leave_cond(bool cond)
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch_cond(cond, false);
    }
}
void
_lotto_region_atomic_enter_task(task_id task)
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch_task(task, true);
    }
}
void
_lotto_region_atomic_leave_task(task_id task)
{
    if (region_preemption_config()->default_on) {
        lotto_region_preemption_switch_task(task, false);
    }
}
void
_lotto_region_nonatomic_enter()
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch(true);
    }
}
void
_lotto_region_nonatomic_leave()
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch(false);
    }
}
void
_lotto_region_nonatomic_enter_cond(bool cond)
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch_cond(cond, true);
    }
}
void
_lotto_region_nonatomic_leave_cond(bool cond)
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch_cond(cond, false);
    }
}
void
_lotto_region_nonatomic_enter_task(task_id task)
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch_task(task, true);
    }
}
void
_lotto_region_nonatomic_leave_task(task_id task)
{
    if (!region_preemption_config()->default_on) {
        lotto_region_preemption_switch_task(task, false);
    }
}

PS_SUBSCRIBE(CAPTURE_EVENT, EVENT_REGION_PREEMPTION, {
    region_preemption_event_t *ev = EVENT_PAYLOAD(event);
    _lotto_region_preemption_switch(ev->in_region);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_MODULE_INTERCEPT, {
    const context_origin *origin = (const context_origin *)md;
    capture_point *cp            = (capture_point *)event;

    if (cp->src_type != EVENT_REGION_PREEMPTION) {
        return PS_OK;
    }

    runtime_ingress_module_submit_auto(origin, cp);
    return PS_OK;
})
