/* Imported from C:\PSC\csp-rs485\tests\host (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#ifndef CSP_RS485_TEST_H
#define CSP_RS485_TEST_H

#include <stddef.h>

typedef void (*test_function_t)(void);

typedef struct {
    const char *suite;
    const char *name;
    test_function_t function;
} test_case_t;

extern int test_current_failed;

void test_fail_size(
    const char *file,
    int line,
    const char *expression,
    size_t expected,
    size_t actual);
void test_fail_true(
    const char *file,
    int line,
    const char *expression);

#define TEST_ASSERT_EQ_SIZE(expected, actual)                                  \
    do {                                                                        \
        const size_t test_expected_ = (size_t) (expected);                       \
        const size_t test_actual_ = (size_t) (actual);                           \
        if (test_expected_ != test_actual_) {                                    \
            test_fail_size(                                                      \
                __FILE__,                                                        \
                __LINE__,                                                        \
                #actual,                                                         \
                test_expected_,                                                  \
                test_actual_);                                                   \
            return;                                                              \
        }                                                                       \
    } while (0)

#define TEST_ASSERT_TRUE(expression)                                            \
    do {                                                                        \
        if (!(expression)) {                                                     \
            test_fail_true(__FILE__, __LINE__, #expression);                     \
            return;                                                              \
        }                                                                       \
    } while (0)

#endif
