#define LOGGER_BLOCK LOGGER_CUR_BLOCK
#include "state.h"
#include <lotto/engine/sequencer.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/sys/assert.h>
#include <lotto/sys/logger_block.h>
#include <lotto/util/macros.h>

STATIC void
_atomic_handle(const context_t *ctx, event_t *e)
{
    ASSERT(e);
    if (e->skip || !atomic_config()->enabled)
        return;

    ASSERT(ctx);
    ASSERT(ctx->id != NO_TASK);

    if (e->readonly)
        return;

    // NOLINTBEGIN(bugprone-branch-clone): Fixme - Fix and remove no lint line
    switch (context_memaccess_event(ctx)) {
        case CONTEXT_MA_BEFORE_AREAD:
        case CONTEXT_MA_BEFORE_AWRITE:
        case CONTEXT_MA_BEFORE_XCHG:
        case CONTEXT_MA_BEFORE_CMPXCHG:
        case CONTEXT_MA_BEFORE_RMW:
        case CONTEXT_MA_BEFORE_FENCE:
            if (!e->is_chpt) {
                e->reason  = REASON_DETERMINISTIC;
                e->is_chpt = true;
            }
            break;

        case CONTEXT_MA_AFTER_AREAD:
        case CONTEXT_MA_AFTER_AWRITE:
        case CONTEXT_MA_AFTER_XCHG:
        case CONTEXT_MA_AFTER_RMW:
        case CONTEXT_MA_AFTER_CMPXCHG_S:
        case CONTEXT_MA_AFTER_CMPXCHG_F:
        case CONTEXT_MA_AFTER_FENCE:
        default:
            break;
            // NOLINTEND(bugprone-branch-clone)
    }
}
REGISTER_SEQUENCER_HANDLER(_atomic_handle)
