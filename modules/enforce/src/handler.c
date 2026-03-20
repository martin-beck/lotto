#define LOGGER_BLOCK LOGGER_CUR_BLOCK
#include <lotto/base/marshable.h>
#include <lotto/base/stable_address.h>
#include <lotto/base/string.h>
#include <lotto/engine/catmgr.h>
#include <lotto/engine/dispatcher.h>
#include <lotto/engine/prng.h>
#include <lotto/engine/pubsub.h>
#include <lotto/engine/state.h>
#include <lotto/modules/enforce/state.h>
#include <lotto/runtime/context_payload.h>
#include <lotto/runtime/memaccess_payload.h>
#include <lotto/sys/assert.h>
#include <lotto/sys/logger_block.h>
#include <lotto/sys/stdio.h>
#include <lotto/sys/stdlib.h>
#include <lotto/sys/string.h>
#include <lotto/sys/time.h>
#include <lotto/util/macros.h>
#include <lotto/util/once.h>

#define PASTE(a, b) a##b
#define EQUAL(x)    (enforce_state()->ctx.PASTE(, x) == ctx->PASTE(, x))
#define EQUAL_PC                                                               \
    (pc = stable_address_get(ctx->pc,                                          \
                             sequencer_config()->stable_address_method),       \
     stable_address_equals(&enforce_state()->pc, &pc))
#define EQUAL_DATA                                                             \
    (sys_memcmp(enforce_state()->data, (char *)context_memaccess_addr(ctx),    \
                context_memaccess_size(ctx)) == 0 &&                           \
     (context_memaccess_size(ctx) == ENFORCE_DATA_SIZE ||                      \
      (((char *)context_memaccess_addr(ctx))[context_memaccess_size(ctx)] ==   \
           0 &&                                                                \
       sys_memcmp((char *)context_memaccess_addr(ctx) +                        \
                      context_memaccess_size(ctx),                             \
                  (char *)context_memaccess_addr(ctx) +                        \
                      context_memaccess_size(ctx) + 1,                         \
                  ENFORCE_DATA_SIZE - context_memaccess_size(ctx) - 1) == 0)))
#define EQUAL_SEED (enforce_state()->seed == prng_seed())
#define MODE(x)    (enforce_modes_has(enforce_config()->modes, ENFORCE_MODE_##x))
#define EQUAL_ADDR (enforce_state()->addr == context_memaccess_addr(ctx))

static arg_t
_read_val(const arg_t *ptr, size_t width)
{
    uintptr_t p = ptr->value.ptr;
    ASSERT(ptr->width == ARG_PTR);
    arg_t v = {.width = (enum arg_width)width, .value = {0}};
    switch (width) {
        case 8:
            v.value.u64 = *(uint64_t *)p;
            break;
        case 4:
            v.value.u32 = *(uint32_t *)p;
            break;
        case 2:
            v.value.u16 = *(uint16_t *)p;
            break;
        case 1:
            v.value.u8 = *(uint8_t *)p;
            break;
        default:
            logger_fatalf("unexpected width %u\n", ptr->width);
    }
    return v;
}

bool
_as_expected(const context_t *ctx)
{
    stable_address_t pc;
    context_memaccess_event_t ma = context_memaccess_event(ctx);
    bool has_memaccess           = ma != CONTEXT_MA_NONE;
    if ((MODE(TID) && !EQUAL(id)) || (MODE(CAT) && !EQUAL(cat)) ||
        (MODE(PC) && !EQUAL_PC) ||
        (MODE(ADDRESS) && has_memaccess && !EQUAL_ADDR) ||
        (MODE(SEED) && !EQUAL_SEED))
        return false;

    switch (ma) {
        case CONTEXT_MA_BEFORE_READ:
        case CONTEXT_MA_BEFORE_AREAD:
        case CONTEXT_MA_BEFORE_WRITE:
        case CONTEXT_MA_BEFORE_AWRITE:
            if (MODE(DATA)) {
                arg_t p = arg_ptr((void *)context_memaccess_addr(ctx));
                arg_t a = _read_val(&p, context_memaccess_size(ctx));
                if (enforce_state()->val.value.u64 != a.value.u64) {
                    return false;
                }
            }
            break;

        case CAT_ENFORCE:
            if (!MODE(CUSTOM)) {
                break;
            }
            ASSERT(context_memaccess_size(ctx) <= ENFORCE_DATA_SIZE);
            if (!EQUAL_DATA) {
                return false;
            }
            break;

        default:
            break;
    }

    return true;
}

#define REPORT_CTX(fmt, F, x)                                                  \
    REPORT(fmt, F, x, enforce_state()->ctx.PASTE(, x), ctx->PASTE(, x))

#define REPORT(fmt, F, n, x, y)                                                \
    do {                                                                       \
        logger_errorf("MISMATCH [field: %s, expected: " fmt ", actual: " fmt   \
                      "]\n",                                                   \
                      #n, F(x), F(y));                                         \
    } while (0)

#define _(X) X

static void
_report(const context_t *ctx)
{
    category_t cat = context_effective_category(ctx);
    context_memaccess_event_t ma = context_memaccess_event(ctx);
    bool has_memaccess           = ma != CONTEXT_MA_NONE;
    stable_address_t pc;
    if (!EQUAL(id))
        REPORT_CTX("%lu", _, id);
    if (!EQUAL(cat))
        REPORT_CTX("%s", category_str, cat);
    if (MODE(ADDRESS) && has_memaccess && !EQUAL_ADDR)
        REPORT("%lx", _, addr, enforce_state()->addr, context_memaccess_addr(ctx));
    if (MODE(DATA) &&
        (ma == CONTEXT_MA_BEFORE_READ || ma == CONTEXT_MA_BEFORE_AREAD)) {
        arg_t p = arg_ptr((void *)context_memaccess_addr(ctx));
        arg_t a = _read_val(&p, context_memaccess_size(ctx));
        if (enforce_state()->val.value.u64 != a.value.u64) {
            logger_errorf("MISMATCH [field: val, expected: %lu, actual: %lu]\n",
                          enforce_state()->val.value.u64, a.value.u64);
        }
    }
    if (MODE(PC) && !EQUAL_PC)
        REPORT_CTX("%p", (void *), pc);

    if (cat == CAT_ENFORCE && has_memaccess && !EQUAL_DATA) {
        struct value val = on();
        LOTTO_PUBLISH(EVENT_ENFORCE__VIOLATED, val);
        logger_errorf("MISMATCH [field: enforce, expected: ");
        for (size_t i = 0; i < ENFORCE_DATA_SIZE; i++) {
            logger_errorf("%2.2x", enforce_state()->data[i]);
        }
        logger_errorf(", actual: ");
        for (size_t i = 0; i < context_memaccess_size(ctx); i++) {
            logger_errorf("%2.2x",
                          *((unsigned char *)context_memaccess_addr(ctx) + i));
        }
        logger_errorf("]\n");
    }

    if (MODE(SEED) && !EQUAL_SEED) {
        REPORT("%lu", _, seed, enforce_state()->seed, prng_seed());
    }
}

LOTTO_ADVERTISE_TYPE(EVENT_ENFORCE__VIOLATED)

void
_save(const context_t *ctx, const event_t *e)
{
    context_memaccess_event_t ma = context_memaccess_event(ctx);
    bool has_memaccess           = ma != CONTEXT_MA_NONE;

    switch (context_effective_category(ctx)) {
        case CAT_BEFORE_READ:
        case CAT_BEFORE_AREAD:
        case CAT_BEFORE_WRITE:
        case CAT_BEFORE_AWRITE:
            if (MODE(DATA)) {
                arg_t p = arg_ptr((void *)context_memaccess_addr(ctx));
                enforce_state()->val =
                    _read_val(&p, context_memaccess_size(ctx));
            }
            break;

        case CAT_ENFORCE:
            if (!MODE(CUSTOM)) {
                break;
            }
            ASSERT(context_memaccess_size(ctx) <= ENFORCE_DATA_SIZE);
            sys_memcpy(enforce_state()->data,
                       (char *)context_memaccess_addr(ctx),
                       context_memaccess_size(ctx));
            sys_memset(enforce_state()->data + context_memaccess_size(ctx), 0,
                       ENFORCE_DATA_SIZE - context_memaccess_size(ctx));
            break;

        default:
            break;
    }
    enforce_state()->clk = e->clk;
    if (MODE(CAT) || MODE(TID) || MODE(ADDRESS)) {
        enforce_state()->ctx = *ctx;
        if (MODE(ADDRESS) && has_memaccess) {
            enforce_state()->addr = context_memaccess_addr(ctx);
        }
    }
    if (MODE(PC)) {
        enforce_state()->pc = stable_address_get(
            ctx->pc, sequencer_config()->stable_address_method);
    }
    if (MODE(SEED)) {
        enforce_state()->seed = prng_seed();
    }
}

static void
check_aslr()
{
    char *randomize_va_space = "/proc/sys/kernel/randomize_va_space";
    FILE *fp                 = sys_fopen(randomize_va_space, "r");
    if (!fp) {
        logger_warnf("Can't read ASLR status\n");
    } else {
        char aslr[1];
        sys_fread(aslr, 1, 1, fp);
        if (aslr[0] != '0' &&
            (MODE(PC) || MODE(ADDRESS) || MODE(DATA) || MODE(CUSTOM)) &&
            sequencer_config()->stable_address_method ==
                STABLE_ADDRESS_METHOD_NONE) {
            logger_warnf("ASLR enabled but no stable address method\n");
        }
    }
}

void
_handle(const context_t *ctx, event_t *cp)
{
    once(check_aslr());
    if (enforce_config()->modes == ENFORCE_MODE_NONE)
        return;
    if (MODE(CUSTOM) && context_effective_category(ctx) == CAT_ENFORCE) {
        cp->should_record = true;
    }
    if (cp->replay && cp->clk == enforce_state()->clk) {
        if (!_as_expected(ctx)) {
            logger_errorf(
                "Replay mismatch! cappt = [clk: %lu, id: %lu, cat: %s, pc: "
                "%p]\n",
                cp->clk, ctx->id, category_str(context_effective_category(ctx)),
                (void *)ctx->pc);
            _report(ctx);
            logger_fatalf("unexpected capture point\n");
            sys_abort();
        }
    }
    _save(ctx, cp);
}
REGISTER_SEQUENCER_HANDLER(_handle)
