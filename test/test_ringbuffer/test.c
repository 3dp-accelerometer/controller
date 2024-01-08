#include <errno.h>
#include <inttypes.h>
// #include <ringbuffer.h>
#include "../../lib/ringbuffer/src/ringbuffer.h"
#include <stdbool.h>
#include <stdio.h>
#include <unity.h>

struct Foo {
  uint8_t data;
} __attribute__((packed));

#define DECLARE_BUFFER_CAPACITY1                                               \
  struct Foo storage[1] = {0};                                                 \
  struct Ringbuffer buffer;                                                    \
  Ringbuffer_init(&buffer, (uint8_t *)storage, 1, sizeof(struct Foo))

#define DECLARE_BUFFER_CAPACITY3                                               \
  struct Foo storage[3] = {0};                                                 \
  struct Ringbuffer buffer;                                                    \
  Ringbuffer_init(&buffer, (uint8_t *)storage, 3, sizeof(struct Foo))

#define DECLARE_BUFFER_CAPACITY65535                                           \
  struct Foo storage[65535] = {0};                                             \
  struct Ringbuffer buffer;                                                    \
  Ringbuffer_init(&buffer, (uint8_t *)storage, 65535, sizeof(struct Foo))

void test_cap1_empty_isEmptyNotFull() {
  DECLARE_BUFFER_CAPACITY1;

  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));
}

void test_cap1_full() {
  DECLARE_BUFFER_CAPACITY1;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));
}

void test_cap3_empty_isEmptyNotFull() {
  DECLARE_BUFFER_CAPACITY3;

  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));
}

void test_cap3_notEmptyNotFull_isNotEmptyNotFull() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));
}

void test_cap3_full_isNotEmptyButFull() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));
}

void test_cap3_overflow() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));
  TEST_ASSERT_EQUAL(-EOVERFLOW, Ringbuffer_put(&buffer, (uint8_t *)&item));
}

void test_cap3_empty_takeItem() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(-ENODATA, Ringbuffer_take(&buffer, (uint8_t *)&item));
}

void test_cap3_notEmptyNotFull_takeItem() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));
  TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item));
}

void test_cap3_full_takeItem() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));
  TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item));
}

void test_cap3_putAndOverflow() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item = {.data = 42};

  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(-EOVERFLOW, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(-EOVERFLOW, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));

  TEST_ASSERT_EQUAL(-EOVERFLOW, Ringbuffer_put(&buffer, (uint8_t *)&item));

  TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));
}

void test_cap3_putAndTakeNoOverflow() {
  DECLARE_BUFFER_CAPACITY3;
  struct Foo item1 = {.data = 42};
  struct Foo item2 = {.data = 42};

  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
  TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

  for (int i = 0; i < 100; i++) {
    // 0 items
    TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item1));
    // 1 item

    TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

    // 1 item
    TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item1));
    // 2 items

    TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

    // 2 items
    TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item1));
    // 3 items

    TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(true, Ringbuffer_isFull(&buffer));

    // 3 items
    TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item2));
    // 2 items

    TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

    // 2 items
    TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item2));
    // 1 item

    TEST_ASSERT_EQUAL(false, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));

    // 1 item
    TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item2));
    // 0 items

    TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
    TEST_ASSERT_EQUAL(false, Ringbuffer_isFull(&buffer));
  }
}

void test_cap65535_beyondLimitsAndAbove() {
  DECLARE_BUFFER_CAPACITY65535;
  struct Foo item = {.data = 42};

  for (uint32_t i = 0; i < (65535 + 100); i++) {
    char msg[16];
    snprintf(msg, 16, "index=%d", i);
    item.data = i % 255;

    int expectedReturn = (i < 65535) ? 0 : -EOVERFLOW;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        expectedReturn, Ringbuffer_put(&buffer, (uint8_t *)&item), msg);
  }

  for (uint32_t i = 0; i < (65535 + 100); i++) {
    char msg[16];
    snprintf(msg, 16, "index=%d", i);
    item.data = 42;

    int expectedReturn = (i <= 65534) ? 0 : -ENODATA;
    TEST_ASSERT_EQUAL_INT_MESSAGE(
        expectedReturn, Ringbuffer_take(&buffer, (uint8_t *)&item), msg);

    if (expectedReturn == 0)
      TEST_ASSERT_EQUAL_INT_MESSAGE(i % 255, item.data, msg);
    else
      TEST_ASSERT_EQUAL_INT_MESSAGE(42, item.data, msg);
  }
}

void test_cap65535_movingWindowBeyondLimits() {
  DECLARE_BUFFER_CAPACITY65535;
  struct Foo item;

  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));

  // introduce moving window of 3 items
  item.data = 0;
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  item.data = 1;
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));
  item.data = 2;
  TEST_ASSERT_EQUAL(0, Ringbuffer_put(&buffer, (uint8_t *)&item));

  // put and take items while the index overflows 4 times
  for (uint32_t i = 3; i < (4 * 65535); i++) {
    char msg[16];
    snprintf(msg, 16, "index=%d", i);
    item.data = i % 255;

    TEST_ASSERT_EQUAL_INT_MESSAGE(0, Ringbuffer_put(&buffer, (uint8_t *)&item),
                                  msg);
    item.data = 42;
    TEST_ASSERT_EQUAL_INT_MESSAGE(0, Ringbuffer_take(&buffer, (uint8_t *)&item),
                                  msg);
    TEST_ASSERT_EQUAL_INT_MESSAGE((i - 3) % 255, item.data, msg);
  }

  // consume remaining window of 3 items
  item.data = 42;
  TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL((2 * 65535 - 3) % 255, item.data);

  item.data = 42;
  TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL((2 * 65535 - 2) % 255, item.data);

  item.data = 42;
  TEST_ASSERT_EQUAL(0, Ringbuffer_take(&buffer, (uint8_t *)&item));
  TEST_ASSERT_EQUAL((2 * 65535 - 1) % 255, item.data);

  // assert buffer is empty
  TEST_ASSERT_EQUAL(true, Ringbuffer_isEmpty(&buffer));
}

int tests() {
  UNITY_BEGIN();
  RUN_TEST(test_cap1_empty_isEmptyNotFull);
  RUN_TEST(test_cap1_full);
  RUN_TEST(test_cap3_empty_isEmptyNotFull);
  RUN_TEST(test_cap3_notEmptyNotFull_isNotEmptyNotFull);
  RUN_TEST(test_cap3_full_isNotEmptyButFull);
  RUN_TEST(test_cap3_overflow);
  RUN_TEST(test_cap3_empty_takeItem);
  RUN_TEST(test_cap3_notEmptyNotFull_takeItem);
  RUN_TEST(test_cap3_putAndOverflow);
  RUN_TEST(test_cap3_putAndTakeNoOverflow);
  RUN_TEST(test_cap65535_beyondLimitsAndAbove);
  RUN_TEST(test_cap65535_movingWindowBeyondLimits);
  return UNITY_END();
}

void setUp() {}

void tearDown() {}

#include "../utils/run-tests.h"
