# Ingress Refactor Notes

## Current Layering

The intended event flow is:

1. `INTERCEPT_*`
   Raw source events emitted by Dice interceptors or custom Lotto interceptors.

2. `CAPTURE_*`
   The same events after Dice `self` has attached TLS / task metadata.

3. `INGRESS_*`
   Lotto-normalized semantic events.
   These are published on:
   - `CHAIN_INGRESS`
   - `CHAIN_INGRESS_BEFORE`
   - `CHAIN_INGRESS_AFTER`

4. Existing runtime bridge
   The current system still converts ingress back into the old
   `runtime_ingress()`, `runtime_ingress_before()`, and
   `runtime_ingress_after()` APIs.

This means `runtime_ingress*` is now a compatibility layer, not the desired
public boundary.

## Current Data Model

### `context_t`

`context_t` is still the compatibility carrier used by the old runtime path.
It currently contains:

- Dice metadata prefix `metadata_t _`
- `metadata_t *self` to keep the original Dice `self` metadata object
- `cat`
- task ids
- `pc`, `func`, `func_addr`
- `args[]`

This is still too overloaded, but it is now mostly on the old side of the
bridge.

### `capture_point`

`capture_point` is the new semantic ingress payload.

Current shape:

- `src_type`
- pointer-only union
- generic fallback `void *payload`
- a few core pointer payloads for runtime-owned events

The important rule is:

- ingress chain event type is the semantic event kind
- `capture_point.src_type` preserves the finer source kind when needed

## Naming / Ownership Rules

### Chains

- `INTERCEPT_*` is raw interception ABI
- `CAPTURE_*` is self-enriched raw capture
- `INGRESS_*` is Lotto semantic ingress
- sequencer chains come later

### Event ownership

- runtime-owned ingress events live in
  `include/lotto/runtime/ingress_events.h`
- module-owned semantic events live in each module under
  `modules/<name>/include/lotto/modules/<name>/events.h`

This is intentional:

- the core should not need to know module-specific semantic ids
- module-specific events should travel through `EVENT_MODULE_INTERCEPT`
  when they are not runtime/core events

## What Has Already Been Moved

### Custom interceptor side

Most custom interceptors now publish raw events on `INTERCEPT_*` first rather
than calling the runtime directly.

### Module-owned semantic ingress

The following module families already use `INGRESS_*` with local compatibility
bridges:

- mutex
- evec
- deadlock resource events
- priority
- task velocity
- region preemption
- custom yield (`lotto_yield`)
- poll
- time-yield
- order
- fork
- rogue
- Rust await / spin-loop hooks
- rwlock
- tsan
- cxa
- join
- `sched_yield`

### Core/runtime-owned ingress

The runtime bridge currently handles core ingress events such as:

- task init
- task fini
- task create
- call
- detach
- key create
- key delete
- set specific

## Important Current Boundary

The intended public/non-legacy boundary is:

- producers publish semantic events into `INGRESS_*`

The current compatibility boundary is:

- `INGRESS_*` subscribers still call `runtime_ingress*`

This is deliberate staging. The mediator has intentionally not been refactored
yet while the ingress edge is still being stabilized.

## Why `category_t` Still Exists

`category_t` is no longer the desired semantic boundary, but it is still needed
because:

- handlers dispatch on `ctx->cat`
- engine/sequencer still reason over `CAT_*`
- the old runtime bridge still reconstructs `context_t`

So `cat` is currently a compatibility field on the old side of ingress.

## Planned Direction

### Near-term

1. Finish moving all non-QEMU capture subscribers to `INGRESS_*`
2. Keep module-owned/local ingress bridges where the core should not know the
   event details
3. Keep runtime-owned bridges only for actual core/runtime events

Status:

- all non-QEMU `CAPTURE_*` subscribers now publish into `INGRESS_*`
- remaining direct `runtime_ingress*` calls are only:
  - runtime/core ingress bridges
  - module-local ingress compatibility bridges
  - the legacy runtime ingress implementation itself

### Mid-term

1. Reduce direct dependence on `context_t.cat`
2. Move handler/sequencer boundaries away from the current implicit
   `context_t` + `event_t` pairing
3. Introduce explicit sequencer-facing messages instead of smuggling context via
   Dice metadata

### Later

1. Replace `context_t` with a narrower `context`
2. Keep semantic meaning in ingress event type plus `capture_point`
3. Eventually remove `category_t` once the old bridge is gone

## QEMU

QEMU is intentionally excluded from the current refactor wave.

It synthesizes Lotto-side contexts directly and should be migrated separately
once the non-QEMU ingress model is settled.
