#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/arg.h>
#include <lotto/base/category.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

static inline context_t
bridge_context_(const context_t *origin, category_t cat)
{
    ASSERT(origin != NULL);
    context_t ctx = *origin;
    ctx.cat       = cat;
    return ctx;
}

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_KEY_CREATE, {
    const context_t *origin          = (const context_t *)md;
    capture_point *cp                = (capture_point *)event;
    capture_key_create_event *kcev   = cp->key_create;
    context_t ctx = bridge_context_(origin, CAT_KEY_CREATE);
    ctx.args[0]   = arg_ptr(kcev->key);
    ctx.args[1]   = arg_ptr(kcev->destructor);
    runtime_ingress(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_TASK_INIT, {
    const context_t *origin        = (const context_t *)md;
    capture_point *cp              = (capture_point *)event;
    capture_task_init_event *tiev  = cp->task_init;
    context_t ctx                  = bridge_context_(origin, CAT_TASK_INIT);
    ctx.args[0]                    = arg(uintptr_t, tiev->thread);
    ctx.args[1]                    = arg(bool, tiev->detached);
    runtime_ingress(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_TASK_FINI, {
    const context_t *origin        = (const context_t *)md;
    capture_point *cp              = (capture_point *)event;
    capture_task_fini_event *tfev  = cp->task_fini;
    context_t ctx                  = bridge_context_(origin, CAT_TASK_FINI);
    ctx.args[0]                    = arg_ptr(tfev->ptr);
    runtime_ingress(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_TASK_CREATE, {
    const context_t *origin          = (const context_t *)md;
    capture_point *cp                = (capture_point *)event;
    capture_task_create_event *tcev  = cp->task_create;
    context_t ctx                    = bridge_context_(origin, CAT_TASK_CREATE);
    ctx.args[0]                      = arg_ptr(tcev->thread);
    ctx.args[1]                      = arg_ptr(tcev->attr);
    ctx.args[2]                      = arg_ptr(tcev->run);
    (void)runtime_ingress_before(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_TASK_CREATE, {
    const context_t *origin = (const context_t *)md;
    runtime_ingress_after(origin->func);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_CALL, {
    const context_t *origin = (const context_t *)md;
    context_t ctx           = bridge_context_(origin, CAT_CALL);
    (void)runtime_ingress_before(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_CALL, {
    const context_t *origin = (const context_t *)md;
    runtime_ingress_after(origin->func);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_DETACH, {
    const context_t *origin      = (const context_t *)md;
    capture_point *cp            = (capture_point *)event;
    capture_detach_event *dev    = cp->detach;
    context_t ctx                = bridge_context_(origin, CAT_DETACH);
    ctx.args[0]                  = arg(uint64_t, dev->thread);
    ctx.args[1]                  = arg_ptr(dev->ret);
    runtime_ingress(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_KEY_DELETE, {
    const context_t *origin          = (const context_t *)md;
    capture_point *cp                = (capture_point *)event;
    capture_key_delete_event *kdev   = cp->key_delete;
    context_t ctx = bridge_context_(origin, CAT_KEY_DELETE);
    ctx.args[0]   = arg_ptr(&kdev->key);
    runtime_ingress(&ctx);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_SET_SPECIFIC, {
    const context_t *origin             = (const context_t *)md;
    capture_point *cp                   = (capture_point *)event;
    capture_set_specific_event *ssev    = cp->set_specific;
    context_t ctx = bridge_context_(origin, CAT_SET_SPECIFIC);
    ctx.args[0]   = arg_ptr(&ssev->key);
    ctx.args[1]   = arg_ptr(ssev->value);
    runtime_ingress(&ctx);
    return PS_OK;
})
