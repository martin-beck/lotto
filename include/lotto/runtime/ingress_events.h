/**
 * @file ingress_events.h
 * @brief Lotto ingress semantic event identifiers.
 */
#ifndef LOTTO_RUNTIME_INGRESS_EVENTS_H
#define LOTTO_RUNTIME_INGRESS_EVENTS_H

#define EVENT_TASK_INIT     170
#define EVENT_TASK_FINI     171
#define EVENT_TASK_CREATE   172
#define EVENT_CALL          173
#define EVENT_TASK_DETACH   174
#define EVENT_KEY_CREATE    165
#define EVENT_KEY_DELETE    166
#define EVENT_SET_SPECIFIC  167
#define EVENT_MODULE_INTERCEPT 168
#define EVENT_BEFORE_READ      177
#define EVENT_BEFORE_WRITE     178
#define EVENT_BEFORE_AREAD     179
#define EVENT_BEFORE_AWRITE    180
#define EVENT_BEFORE_RMW       181
#define EVENT_BEFORE_XCHG      182
#define EVENT_BEFORE_CMPXCHG   183
#define EVENT_BEFORE_FENCE     184
#define EVENT_AFTER_AREAD      185
#define EVENT_AFTER_AWRITE     186
#define EVENT_AFTER_RMW        187
#define EVENT_AFTER_XCHG       188
#define EVENT_AFTER_CMPXCHG_S  189
#define EVENT_AFTER_CMPXCHG_F  190
#define EVENT_AFTER_FENCE      191
#define EVENT_FUNC_ENTRY       192
#define EVENT_FUNC_EXIT        193

#endif
