/**
 * \file buffer.h
 *
 * Naive ringbuffer implementation for storing items in slots of constant size.
 */

#pragma once

#include <inttypes.h>
#include <stdbool.h>

/**
 * Buffer index for Ringbuffer keeping track of start and end pointer.
 */
struct Ringbuffer_Index {
  uint16_t begin;
  uint16_t end;
  uint16_t capacity;   ///< maximum number of items
  uint16_t itemsCount; ///< currently stored items

  bool isFull;
  bool isEmpty;
  uint8_t itemSizeBytes; ///< size of one item in bytes
};

/**
 * Buffer for storing generic types in a circular manner.
 *
 * Example:
 * \code
 * #define CAPACITY 16
 *
 * struct Foo {
 *   uint16_t data;
 * };
 *
 * struct Foo storage[CAPACITY];
 * struct Ringbuffer buffer;
 * struct Ringbuffer_init(
 *   &buffer,
 *   (uint8_t*)&storage,
 *   CAPACITY,
 *   sizeof(struct Foo));
 *
 * struct Foo item = {};
 * Ringbuffer_put(&buffer, (uint8_t *)&item);
 *
 * \endcode
 */
struct Ringbuffer {
  struct Ringbuffer_Index index;
  uint8_t *storage; ///< index offset is mapped to
                    ///< n*sizeof(Ringbuffer_Index.itemSizeBytes);
                    ///< must be uint8_t for proper pointer arithmetics
};

int RingbufferIndex_init(struct Ringbuffer_Index *index, uint16_t capacity,
                         uint8_t itemSizeBytes);

int Ringbuffer_init(struct Ringbuffer *buffer, uint8_t *storage,
                    uint16_t itemsCount, uint8_t itemSizeBytes);

/**
 * Stores one item to the buffer if possible.
 *
 * @param buffer
 * @param item input data
 * @return -EOVERFLOW if buffer is full, 0 otherwise
 */
int Ringbuffer_put(struct Ringbuffer *buffer, const void *item);

/**
 * Takes one item from the buffer if possible.
 *
 * @param buffer
 * @param item output buffer
 * @return -ENODATA if buffer is empty, 0 otherwise
 */
int Ringbuffer_take(struct Ringbuffer *buffer, void *item);

/**
 * Tests whether the buffer is empty.
 *
 * @param buffer
 * @return true if buffer is empty
 */
bool Ringbuffer_isEmpty(const struct Ringbuffer *buffer);

/**
 * Tests whether the buffer is full.
 *
 * @param buffer
 * @return true if capacity is exhausted
 */
bool Ringbuffer_isFull(const struct Ringbuffer *buffer);

/**
 * Returns amount of stored items (used slots).
 * @param buffer
 * @return number of items stored in buffer; between 0 and 65535
 */
uint16_t Ringbuffer_itemsCount(const struct Ringbuffer *buffer);

/**
 * Invalidates the start/end indices without zeroing out the buffer.
 *
 * @param buffer
 */
void Ringbuffer_reset(struct Ringbuffer *buffer);
