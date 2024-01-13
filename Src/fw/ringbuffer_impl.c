#include "fw/ringbuffer_impl.h"

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
uint8_t ringbufferStorage[RINGBUFFER_STORAGE_SIZE_BYTES] = {0};
