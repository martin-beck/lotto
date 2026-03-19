/**
 * @file context_origin.h
 * @brief Narrow ingress-origin context declarations.
 */
#ifndef LOTTO_RUNTIME_CONTEXT_ORIGIN_H
#define LOTTO_RUNTIME_CONTEXT_ORIGIN_H

#include <stdint.h>

#include <dice/types.h>
#include <lotto/base/task_id.h>

typedef struct context_origin {
    struct metadata _;
    metadata_t *self;
    task_id id;
    task_id vid;
    uintptr_t pc;
#if defined(QLOTTO_ENABLED)
    uint64_t icount;
    uint32_t pstate;
#endif
    const char *func;
    uintptr_t func_addr;
} context_origin;

#define CTX_LEVELS 0

#define ctx_origin(...)                                                        \
    (&(context_origin){                                                        \
        .self      = NULL,                                                     \
        .id        = NO_TASK,                                                  \
        .vid       = NO_TASK,                                                  \
        .pc        = ((uintptr_t)__builtin_return_address(CTX_LEVELS)),        \
        .func      = "UNKNOWN",                                                \
        .func_addr = ((uintptr_t)__builtin_frame_address(CTX_LEVELS)),         \
        __VA_ARGS__})

#define ctx_origin_pc(...)                                                     \
    (&(context_origin){.self = NULL,                                           \
                       .id   = NO_TASK,                                        \
                       .vid  = NO_TASK,                                        \
                       .func = "UNKNOWN",                                      \
                       .func_addr =                                            \
                           ((uintptr_t)__builtin_frame_address(CTX_LEVELS)),   \
                       __VA_ARGS__})

#endif
