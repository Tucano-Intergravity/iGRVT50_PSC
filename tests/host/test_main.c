/* Imported from C:\PSC\csp-rs485\tests\host\test_main.c (authorized subset). */
/* This import makes no assertion about upstream licensing. */
#include "support/host_csp.h"
#include "support/test.h"

#include <csp/csp_error.h>

#include <csp_rs485_link.h>

#include <stdio.h>
#include <string.h>

extern const test_case_t host_profile_tests[];
extern const size_t host_profile_test_count;
extern const test_case_t kiss_encoder_tests[];
extern const size_t kiss_encoder_test_count;
extern const test_case_t link_tx_tests[];
extern const size_t link_tx_test_count;
extern const test_case_t link_rx_tests[];
extern const size_t link_rx_test_count;
extern const test_case_t supervisor_tests[];
extern const size_t supervisor_test_count;
extern const test_case_t freertos_runtime_tests[];
extern const size_t freertos_runtime_test_count;
extern const test_case_t sam_csp_codec_tests[];
extern const size_t sam_csp_codec_test_count;

typedef struct {
    const test_case_t *tests;
    size_t count;
} test_group_t;

int test_current_failed;

void test_fail_size(
    const char *file,
    int line,
    const char *expression,
    size_t expected,
    size_t actual)
{
    test_current_failed = 1;
    fprintf(
        stderr,
        "%s:%d: expected %s to be %zu, got %zu\n",
        file,
        line,
        expression,
        expected,
        actual);
}

void test_fail_true(
    const char *file,
    int line,
    const char *expression)
{
    test_current_failed = 1;
    fprintf(stderr, "%s:%d: expected %s to be true\n", file, line, expression);
}

static int matches_filter(const test_case_t *test, const char *filter)
{
    if ((filter == NULL) || (filter[0] == '\0')) {
        return 1;
    }

    return (strstr(test->suite, filter) != NULL)
        || (strstr(test->name, filter) != NULL);
}

static int run_test(const test_case_t *test)
{
    const int init_result = host_csp_init();
    if (init_result != CSP_ERR_NONE) {
        fprintf(
            stderr,
            "FAIL %s: host_csp_init returned %d\n",
            test->name,
            init_result);
        return 1;
    }

    test_current_failed = 0;
    test->function();
    csp_rs485_link_deinit();
    host_csp_cleanup();

    if (test_current_failed != 0) {
        fprintf(stderr, "FAIL %s\n", test->name);
        return 1;
    }

    printf("PASS %s\n", test->name);
    return 0;
}

int main(int argc, char **argv)
{
    const char *filter = (argc > 1) ? argv[1] : NULL;
    size_t selected = 0U;
    size_t failures = 0U;
    const test_group_t groups[] = {
        {host_profile_tests, host_profile_test_count},
        {kiss_encoder_tests, kiss_encoder_test_count},
        {link_tx_tests, link_tx_test_count},
        {link_rx_tests, link_rx_test_count},
        {supervisor_tests, supervisor_test_count},
        {freertos_runtime_tests, freertos_runtime_test_count},
        {sam_csp_codec_tests, sam_csp_codec_test_count},
    };

    for (size_t group = 0U; group < (sizeof(groups) / sizeof(groups[0])); ++group) {
        for (size_t index = 0U; index < groups[group].count; ++index) {
            const test_case_t *test = &groups[group].tests[index];
            if (!matches_filter(test, filter)) {
                continue;
            }

            ++selected;
            failures += (size_t) run_test(test);
        }
    }

    if (selected == 0U) {
        fprintf(
            stderr,
            "No tests matched filter: %s\n",
            (filter != NULL) ? filter : "(none)");
        return 2;
    }

    printf("%zu tests, %zu failures\n", selected, failures);
    return (failures == 0U) ? 0 : 1;
}
