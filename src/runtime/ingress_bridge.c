#include <dice/module.h>
#include <dice/pubsub.h>
#include <lotto/base/context.h>
#include <lotto/engine/pubsub.h>
#include <lotto/runtime/ingress.h>
#include <lotto/runtime/ingress_events.h>

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_KEY_CREATE, {
    runtime_ingress_event((const context_t *)md, EVENT_KEY_CREATE,
                          (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_TASK_INIT, {
    runtime_ingress_event((const context_t *)md, EVENT_TASK_INIT,
                          (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_TASK_FINI, {
    runtime_ingress_event((const context_t *)md, EVENT_TASK_FINI,
                          (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_TASK_CREATE, {
    (void)runtime_ingress_event_before((const context_t *)md, EVENT_TASK_CREATE,
                                       (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_TASK_CREATE, {
    runtime_ingress_event_after((const context_t *)md, EVENT_TASK_CREATE,
                                (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_BEFORE, EVENT_CALL, {
    (void)runtime_ingress_event_before((const context_t *)md, EVENT_CALL,
                                       (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS_AFTER, EVENT_CALL, {
    runtime_ingress_event_after((const context_t *)md, EVENT_CALL,
                                (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_TASK_DETACH, {
    runtime_ingress_event((const context_t *)md, EVENT_TASK_DETACH,
                          (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_KEY_DELETE, {
    runtime_ingress_event((const context_t *)md, EVENT_KEY_DELETE,
                          (const capture_point *)event);
    return PS_OK;
})

PS_SUBSCRIBE(CHAIN_INGRESS, EVENT_SET_SPECIFIC, {
    runtime_ingress_event((const context_t *)md, EVENT_SET_SPECIFIC,
                          (const capture_point *)event);
    return PS_OK;
})
