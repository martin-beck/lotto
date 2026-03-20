/**
 * @file sequencer.h
 * @brief Engine declarations for sequencer.
 */
#ifndef LOTTO_SEQUENCER_H
#define LOTTO_SEQUENCER_H

#include <stdbool.h>

#include <lotto/base/clk.h>
#include <lotto/base/reason.h>
#include <lotto/base/task_id.h>
#include <lotto/base/tidset.h>
#include <lotto/check.h>
#include <lotto/engine/context.h>
#include <lotto/engine/plan.h>
#include <lotto/engine/pubsub.h>
#include <lotto/runtime/capture_point.h>

enum selector {
    SELECTOR_UNDEFINED = 0,
    SELECTOR_RANDOM,
    SELECTOR_FIRST,
};

typedef struct sequencer_decision {
    const clk_t clk;
    bool is_chpt;
    task_id next;
    bool readonly;
    bool skip;
    tidset_t tset;
    tidset_t unblocked;
    reason_t reason;
    enum selector selector;
    bool should_record;
    bool filter_less;
    bool replay;
    bool (*any_task_filter)(task_id);
} sequencer_decision;

typedef sequencer_decision event_t;

typedef struct sequencer_capture_event {
    const capture_point *cp;
    sequencer_decision *decision;
} sequencer_capture_event;

typedef struct sequencer_resume_event {
    const capture_point *cp;
    sequencer_decision *decision;
} sequencer_resume_event;

typedef void (*handle_f)(const context_t *ctx, sequencer_decision *e);

/**
 * Terminates sequencer
 */
void sequencer_fini(const context_t *ctx, reason_t reason);

/**
 * Returns an action for a given captured context.
 *
 * The action has to be fulfilled following the expected contract.
 */
struct plan sequencer_capture(const context_t *ctx);

/**
 * Informs sequencer that task is resuming after an ACTION_YIELD.
 */
void sequencer_resume(const context_t *ctx);

/**
 * Informs the sequencer that the task has returned from a call.
 */
void sequencer_return(const context_t *ctx);

/**
 * Returns the current clock
 */
clk_t sequencer_get_clk();

#define REGISTER_HANDLER(handle)                                               \
    PS_SUBSCRIBE(CHAIN_SEQUENCER_CAPTURE, EVENT_SEQUENCER_CAPTURE, {           \
        const context_t *ctx = (const context_t *)md;                          \
        sequencer_capture_event *capture_event =                               \
            (sequencer_capture_event *)event;                                  \
        sequencer_decision *e = capture_event->decision;                       \
        handle(ctx, e);                                                        \
        if (e->skip)                                                           \
            return PS_STOP_CHAIN;                                              \
    })

#define REGISTER_HANDLER_EXTERNAL(handle)                                      \
    PS_SUBSCRIBE(CHAIN_SEQUENCER_CAPTURE, EVENT_SEQUENCER_CAPTURE, {           \
        const context_t *ctx = (const context_t *)md;                          \
        sequencer_capture_event *capture_event =                               \
            (sequencer_capture_event *)event;                                  \
        sequencer_decision *e = capture_event->decision;                       \
        if (lotto_loaded())                                                    \
            handle(ctx, e);                                                    \
        if (e->skip)                                                           \
            return PS_STOP_CHAIN;                                              \
    })

#define REGISTER_SEQUENCER_HANDLER(handle)                                     \
    PS_SUBSCRIBE(CHAIN_SEQUENCER_CAPTURE, EVENT_SEQUENCER_CAPTURE, {           \
        const context_t *ctx = (const context_t *)md;                          \
        sequencer_capture_event *capture_event =                               \
            (sequencer_capture_event *)event;                                  \
        sequencer_decision *e = capture_event->decision;                       \
        handle(ctx, e);                                                        \
        if (e->skip)                                                           \
            return PS_STOP_CHAIN;                                              \
    })

#endif
