#include <unity.h>

void test_01() {
  uint8_t half_the_story = 21;
  uint8_t answer_to_ultimate_question = 42;
  TEST_ASSERT_EQUAL(answer_to_ultimate_question,
                    half_the_story + half_the_story);
}

int tests() {
  UNITY_BEGIN();
  RUN_TEST(test_01);
  return UNITY_END();
}

void setUp() {}

void tearDown() {}

#include "../utils/run-tests.h"
