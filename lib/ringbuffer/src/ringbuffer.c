/**
 * \file ringbuffer.c
 *
 * Simple ringbuffer implementation.
 */

#include "ringbuffer.h"
#include <errno.h>
#include <string.h>

/**
 * Advances the end index by one if possible.
 *
 * @param index
 * @return -EOVERFLOW if buffer is already full, 0 otherwise
 */
static int advanceEnd(struct Ringbuffer_Index *index) {
  if (index->isFull) {
    return -EOVERFLOW;
  }

  const uint16_t nextEndIndex = (index->end + 1) % index->capacity;
  index->end = nextEndIndex;
  index->isFull = nextEndIndex == index->begin;

  return 0;
}

/**
 * Advances the begin index by one if possible.
 *
 * @param index
 * @return -ENODATA if buffer is already empty, 0 otherwise
 */
static int advanceBegin(struct Ringbuffer_Index *index) {
  if (index->isEmpty) {
    return -ENODATA;
  }

  const uint16_t nextBeginIndex = (index->begin + 1) % index->capacity;
  index->begin = nextBeginIndex;
  index->isEmpty = (index->begin == index->end);

  return 0;
}

/**
 * Retrieves the item at the given index.
 *
 * @param buffer
 * @param index
 * @return pointer to the item
 */
static uint8_t *itemAtIndex(struct Ringbuffer *buffer, uint16_t index) {
  return buffer->storage + (index * buffer->index.itemSizeBytes);
}

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
int RingbufferIndex_init(struct Ringbuffer_Index *index, uint16_t capacity,
                         uint8_t itemSizeBytes) {
  if (NULL == index) {
    return -EINVAL;
  }

  index->begin = 0;
  index->end = 0;
  index->capacity = capacity;
  index->isFull = false;
  index->isEmpty = true;
  index->itemSizeBytes = itemSizeBytes;

  return 0;
}

int Ringbuffer_init(struct Ringbuffer *buffer, uint8_t *storage,
                    uint16_t itemsCount, uint8_t itemSizeBytes) {
  if (NULL == buffer) {
    return -EINVAL;
  }

  if (0 != RingbufferIndex_init(&buffer->index, itemsCount, itemSizeBytes)) {
    return -EINVAL;
  }

  buffer->storage = storage;
  return 0;
}

int Ringbuffer_put(struct Ringbuffer *buffer, const void *item) {
  if (Ringbuffer_isFull(buffer)) {
    return -EOVERFLOW;
  }

  buffer->index.isEmpty = false;
  memcpy(itemAtIndex(buffer, buffer->index.end), item,
         buffer->index.itemSizeBytes);
  advanceEnd(&buffer->index);
  buffer->index.itemsCount++;

  return 0;
}

int Ringbuffer_take(struct Ringbuffer *buffer, void *item) {
  if (Ringbuffer_isEmpty(buffer)) {
    return -ENODATA;
  }

  buffer->index.isFull = false;
  memcpy(item, itemAtIndex(buffer, buffer->index.begin),
         buffer->index.itemSizeBytes);
  advanceBegin(&buffer->index);
  buffer->index.itemsCount--;

  return 0;
}

bool Ringbuffer_isEmpty(const struct Ringbuffer *buffer) {
  return buffer->index.isEmpty;
}

bool Ringbuffer_isFull(const struct Ringbuffer *buffer) {
  return buffer->index.isFull;
}

uint16_t Ringbuffer_itemsCount(const struct Ringbuffer *buffer) {
  return buffer->index.itemsCount;
}

void Ringbuffer_reset(struct Ringbuffer *buffer) {
  buffer->index.begin = 0;
  buffer->index.end = 0;
  buffer->index.itemsCount = 0;
}
