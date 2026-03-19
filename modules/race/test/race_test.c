#include <dice/events/memaccess.h>
#include <lotto/engine/dispatcher.h>
#include <lotto/engine/pubsub.h>
#include <lotto/engine/statemgr.h>
#include <lotto/modules/race/race_result.h>
#include <lotto/runtime/capture_point.h>
#include <lotto/runtime/ingress_events.h>
#include <lotto/sys/ensure.h>
#include <lotto/sys/string.h>
race_t race_check(const context_t *ctx, clk_t clk);

#define A(V) ((uintptr_t)(V))
#define I1   A(1)
#define I2   A(2)
#define I3   A(3)

#define A1 A(101)
#define A2 A(102)
#define A3 A(103)
#define A4 A(104)

void
add_ichpt(uintptr_t addr)
{
}

/*******************************************************************************
 * simple test for adding and checkign ichpts
 ******************************************************************************/
typedef struct {
    context_t ctx;
    race_t race;
    bool end;
} call_t;
#define END                                                                    \
    (call_t)                                                                   \
    {                                                                          \
        .end = true                                                            \
    }

#define race(a, i1, i2)                                                        \
    (race_t)                                                                   \
    {                                                                          \
        .addr = (a), .loc1 = (struct race_loc){.pc = (i1)},                    \
        .loc2 = (struct race_loc){.pc = (i2)},                                 \
    }

#define ctx_read(ID, ADDR, PC)                                                 \
    (context_t)                                                                \
    {                                                                          \
        .id = (ID),                                                            \
        .vid = NO_TASK,                                                        \
        .pc = (PC),                                                            \
        .phase = CONTEXT_PHASE_BEFORE,                                         \
        .type = EVENT_BEFORE_READ, .src_type = EVENT_MA_READ,                  \
        .cp = &(capture_point){                                                \
            .src_type = EVENT_MA_READ,                                         \
            .payload = &(struct ma_read_event){.addr = (void *)(ADDR),         \
                                               .size = sizeof(uintptr_t)}},    \
    }

#define ctx_write(ID, ADDR, PC)                                                \
    (context_t)                                                                \
    {                                                                          \
        .id = (ID),                                                            \
        .vid = NO_TASK,                                                        \
        .pc = (PC),                                                            \
        .phase = CONTEXT_PHASE_BEFORE,                                         \
        .type = EVENT_BEFORE_WRITE, .src_type = EVENT_MA_WRITE,                \
        .cp = &(capture_point){                                                \
            .src_type = EVENT_MA_WRITE,                                        \
            .payload = &(struct ma_write_event){.addr = (void *)(ADDR),        \
                                                .size = sizeof(uintptr_t)}},   \
    }

#define NORACE race(0, 0, 0)

void
test_add()
{
    task_id t1 = 1;
    task_id t2 = 2;

    call_t calls[] = {
        {ctx_read(t1, A1, I1), NORACE},
        {ctx_write(t2, A2, I2), NORACE},
        END,
    };

    for (call_t *c = calls; !c->end; c++) {
        race_t r = race_check(&c->ctx, 0);
        ENSURE(r.addr == c->race.addr);
        ENSURE(r.loc1.pc == c->race.loc2.pc);
    }
}
/*******************************************************************************
 * test saving and restoring the list of ichpts
 ******************************************************************************/

#define BUF_SIZE 2048

char test_buf[BUF_SIZE];
size_t test_write_count;
size_t test_read_count;

size_t
_buf_write(void *arg, const char *buf, size_t size)
{
    (void)arg;
    char *dst = test_buf + test_write_count;
    ENSURE((test_write_count + size + 1) <= BUF_SIZE);
    sys_memcpy(dst, buf, size);
    dst[size] = '\0';
    test_write_count += size;
    return size;
}

size_t
_buf_read(void *arg, char *buf, size_t size)
{
    (void)arg;
    ENSURE((test_read_count + size + 1) <= BUF_SIZE);

    if (test_read_count + size > test_write_count)
        size = test_write_count - test_read_count;

    if (size == 0)
        return 0;

    char *src = test_buf + test_read_count;
    sys_memcpy(buf, src, size);

    test_read_count += size;
    return size;
}

#if 0
void
test_save_load()
{
    /* add this ichpts */
    ichpt_reset();
    add_ichpt(I2);
    add_ichpt(I1);
    ENSURE(is_ichpt(I1));
    ENSURE(is_ichpt(I2));
    ENSURE(!is_ichpt(I3));

    /* save ichpts to a file */
    stream_t s = {
        .write = _buf_write,
        .read  = _buf_read,
        .close = NULL,
        .arg   = NULL,
    };
    ichpt_save(s);

    /* clear ichpts and load stream back */
    ichpt_reset();
    ENSURE(!is_ichpt(I1));
    ENSURE(!is_ichpt(I2));
    ichpt_load(s);
    ENSURE(is_ichpt(I1));
    ENSURE(is_ichpt(I2));
    ENSURE(!is_ichpt(I3));
}
#endif

int
main()
{
    START_REGISTRATION_PHASE();
    test_add();
    // test_save_load();
    return 0;
}
