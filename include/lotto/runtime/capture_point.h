/**
 * @file capture_point.h
 * @brief Lotto ingress capture-point declarations.
 */
#ifndef LOTTO_RUNTIME_CAPTURE_POINT_H
#define LOTTO_RUNTIME_CAPTURE_POINT_H

#include <pthread.h>
#include <stdbool.h>

#include <dice/types.h>

typedef struct source_event {
    const void *pc;
} source_event;

typedef struct capture_task_block_event {
    const char *func;
    bool is_call;
} capture_task_block_event;

typedef struct capture_task_init_event {
    uintptr_t thread;
    bool detached;
} capture_task_init_event;

typedef struct capture_task_fini_event {
    void *ptr;
} capture_task_fini_event;

typedef struct capture_task_create_event {
    void *thread;
    const void *attr;
    void *run;
} capture_task_create_event;

typedef struct capture_task_detach_event {
    uintptr_t thread;
    int *ret;
} capture_task_detach_event;

typedef struct capture_key_create_event {
    pthread_key_t *key;
    void (*destructor)(void *);
} capture_key_create_event;

typedef struct capture_key_delete_event {
    pthread_key_t key;
} capture_key_delete_event;

typedef struct capture_set_specific_event {
    pthread_key_t key;
    const void *value;
} capture_set_specific_event;

typedef struct capture_point {
    type_id src_type;
    union {
        void *payload;
        source_event *source;
        capture_task_block_event *task_block;
        capture_task_init_event *task_init;
        capture_task_fini_event *task_fini;
        capture_task_create_event *task_create;
        capture_task_detach_event *task_detach;
        capture_key_create_event *key_create;
        capture_key_delete_event *key_delete;
        capture_set_specific_event *set_specific;
    };
} capture_point;

#endif
