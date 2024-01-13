/**
 * \file ringbuffer_impl.h
 *
 */

#pragma once

#include <host_transport_types.h>
#include <inttypes.h>

struct Ringbuffer_Index;
struct Ringbuffer;

// NOLINTNEXTLINE(modernize-macro-to-enum)
#define RINGBUFFER_STORAGE_ITEM_SIZE_BYTES                                     \
  (sizeof(struct Transport_Header) + sizeof(struct TransportTx_Acceleration))

// NOLINTNEXTLINE(modernize-macro-to-enum)
#define RINGBUFFER_STORAGE_ITEMS 3200

// NOLINTNEXTLINE(modernize-macro-to-enum)
#define RINGBUFFER_STORAGE_SIZE_BYTES                                          \
  (RINGBUFFER_STORAGE_ITEM_SIZE_BYTES * RINGBUFFER_STORAGE_ITEMS)

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern uint8_t ringbufferStorage[RINGBUFFER_STORAGE_SIZE_BYTES];

#define RINGBUFFER_INDEX_INITIALIZER(CAPACITY, ITEM_SIZE_BYTES)                \
  {                                                                            \
    .begin = 0, .end = 0, .capacity = (CAPACITY), .isFull = false,             \
    .isEmpty = true, .itemSizeBytes = (ITEM_SIZE_BYTES)                        \
  }

#define RINGBUFFER_INITIALIZER(STORAGE_NAME, CAPACITY, ITEM_SIZE_BYTES)        \
  {                                                                            \
    .index = RINGBUFFER_INDEX_INITIALIZER(CAPACITY, ITEM_SIZE_BYTES),          \
    .storage = (STORAGE_NAME)                                                  \
  }

#define RINGBUFFER_DECLARE_INITIALIZER                                         \
  RINGBUFFER_INITIALIZER(ringbufferStorage, RINGBUFFER_STORAGE_ITEMS,          \
                         RINGBUFFER_STORAGE_ITEM_SIZE_BYTES)
