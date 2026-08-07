#include "support/test.h"
#include "fakes/freertos/fake_freertos.h"

#include <FreeRTOS.h>
#include <task.h>

#include <csp/csp.h>
#include <csp/sam_csp_config.h>
#include <csp/samv71_rs485_port.h>

#include <csp_rs485_link.h>

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(SAM_CSP_RUNTIME_HAVE_IMPLEMENTATION)
#include <csp/sam_csp_runtime.h>
#include "csp/sam_csp_runtime_internal.h"
#else
typedef enum {
    SAM_CSP_RUNTIME_OK = 0,
    SAM_CSP_RUNTIME_ERR_CSP_INIT = -1,
    SAM_CSP_RUNTIME_ERR_LINK_INIT = -2,
    SAM_CSP_RUNTIME_ERR_INTERFACE = -3,
    SAM_CSP_RUNTIME_ERR_ROUTE = -4,
    SAM_CSP_RUNTIME_ERR_ROUTER_TASK = -5,
    SAM_CSP_RUNTIME_ERR_SERVICE_SOCKET = -6,
    SAM_CSP_RUNTIME_ERR_SERVICE_BIND = -7,
    SAM_CSP_RUNTIME_ERR_SERVICE_LISTEN = -8,
    SAM_CSP_RUNTIME_ERR_SERVICE_TASK = -9,
} sam_csp_runtime_init_code_t;

typedef struct {
    bool ready;
    int init_code;
    TaskHandle_t router_task;
    TaskHandle_t service_task;
} sam_csp_runtime_status_t;

typedef struct {
    int (*csp_init)(const csp_conf_t *configuration);
    int (*link_init)(const csp_rs485_link_config_t *configuration);
    void (*link_deinit)(void);
    csp_iface_t *(*link_get_interface)(void);
    int (*route_set)(uint8_t destination, csp_iface_t *interface, uint8_t via);
    TaskHandle_t (*task_create_static)(
        TaskFunction_t function,
        const char *name,
        uint32_t stack_words,
        void *argument,
        UBaseType_t priority,
        StackType_t *stack,
        StaticTask_t *task_storage);
    int (*service_prepare)(void);
    void (*link_force_receive)(void);
    void (*report_failure)(int init_code);
} sam_csp_runtime_ops_t;

static int SamCspRuntime_Init(void)
{
    return SAM_CSP_RUNTIME_ERR_CSP_INIT;
}

static bool SamCspRuntime_IsReady(void)
{
    return false;
}

static int SamCspRuntime_GetInitCode(void)
{
    return SAM_CSP_RUNTIME_ERR_CSP_INIT;
}

static void SamCspRuntime_GetStatus(sam_csp_runtime_status_t *status)
{
    if (status != NULL) {
        memset(status, 0, sizeof(*status));
        status->init_code = SAM_CSP_RUNTIME_ERR_CSP_INIT;
    }
}

static void sam_csp_runtime_test_bind_ops(const sam_csp_runtime_ops_t *ops)
{
    (void) ops;
}

static void sam_csp_runtime_test_reset(void)
{
}
#endif

enum {
    EVENT_CSP_INIT = 1,
    EVENT_LINK_INIT,
    EVENT_INTERFACE,
    EVENT_ROUTE,
    EVENT_ROUTER_TASK,
    EVENT_SERVICE_PREPARE,
    EVENT_SERVICE_TASK,
};

typedef enum {
    FAIL_NONE = 0,
    FAIL_CSP_INIT,
    FAIL_LINK_INIT,
    FAIL_INTERFACE,
    FAIL_ROUTE,
    FAIL_ROUTER_TASK,
    FAIL_SERVICE_SOCKET,
    FAIL_SERVICE_BIND,
    FAIL_SERVICE_LISTEN,
    FAIL_SERVICE_TASK,
} runtime_fail_stage_t;

typedef struct {
    int events[12];
    size_t event_count;
    size_t csp_init_calls;
    size_t link_init_calls;
    size_t link_deinit_calls;
    size_t force_receive_calls;
    size_t route_calls;
    size_t task_create_calls;
    size_t service_prepare_calls;
    size_t report_failure_calls;
    int reported_failure;
    csp_conf_t csp_configuration;
    csp_rs485_link_config_t link_configuration;
    uint8_t route_destination;
    uint8_t route_via;
    csp_iface_t *route_interface;
    const char *task_names[2];
    uint32_t task_stacks[2];
    UBaseType_t task_priorities[2];
    TaskHandle_t task_handles[2];
    bool ready_during_last_task_create;
    runtime_fail_stage_t fail_stage;
} fake_runtime_observations_t;

static fake_runtime_observations_t runtime_observed;
static StaticTask_t fake_router_task;
static StaticTask_t fake_service_task;
static uint8_t fake_interface_object;

static void record_runtime_event(int event)
{
    if (runtime_observed.event_count
        < (sizeof(runtime_observed.events) / sizeof(runtime_observed.events[0]))) {
        runtime_observed.events[runtime_observed.event_count] = event;
    }
    ++runtime_observed.event_count;
}

static int runtime_fake_csp_init(const csp_conf_t *configuration)
{
    record_runtime_event(EVENT_CSP_INIT);
    ++runtime_observed.csp_init_calls;
    if (configuration != NULL) {
        runtime_observed.csp_configuration = *configuration;
    }
    return (runtime_observed.fail_stage == FAIL_CSP_INIT)
        ? CSP_ERR_NOMEM : CSP_ERR_NONE;
}

static int runtime_fake_link_init(
    const csp_rs485_link_config_t *configuration)
{
    record_runtime_event(EVENT_LINK_INIT);
    ++runtime_observed.link_init_calls;
    if (configuration != NULL) {
        runtime_observed.link_configuration = *configuration;
    }
    return (runtime_observed.fail_stage == FAIL_LINK_INIT)
        ? CSP_ERR_DRIVER : CSP_ERR_NONE;
}

static void runtime_fake_link_deinit(void)
{
    ++runtime_observed.link_deinit_calls;
}

static csp_iface_t *runtime_fake_get_interface(void)
{
    record_runtime_event(EVENT_INTERFACE);
    if (runtime_observed.fail_stage == FAIL_INTERFACE) {
        return NULL;
    }
    return (csp_iface_t *) (void *) &fake_interface_object;
}

static int runtime_fake_route_set(
    uint8_t destination,
    csp_iface_t *interface,
    uint8_t via)
{
    record_runtime_event(EVENT_ROUTE);
    ++runtime_observed.route_calls;
    runtime_observed.route_destination = destination;
    runtime_observed.route_interface = interface;
    runtime_observed.route_via = via;
    return (runtime_observed.fail_stage == FAIL_ROUTE)
        ? CSP_ERR_INVAL : CSP_ERR_NONE;
}

static TaskHandle_t runtime_fake_task_create(
    TaskFunction_t function,
    const char *name,
    uint32_t stack_words,
    void *argument,
    UBaseType_t priority,
    StackType_t *stack,
    StaticTask_t *task_storage)
{
    const size_t index = runtime_observed.task_create_calls;
    (void) function;
    (void) argument;
    if ((stack == NULL) || (task_storage == NULL)) {
        test_fail_true(__FILE__, __LINE__, "static task storage provided");
        return NULL;
    }
    ++runtime_observed.task_create_calls;
    if (index < 2U) {
        runtime_observed.task_names[index] = name;
        runtime_observed.task_stacks[index] = stack_words;
        runtime_observed.task_priorities[index] = priority;
    }
    if (index == 0U) {
        record_runtime_event(EVENT_ROUTER_TASK);
        if (runtime_observed.fail_stage == FAIL_ROUTER_TASK) {
            return NULL;
        }
        runtime_observed.task_handles[index] = &fake_router_task;
        return &fake_router_task;
    }
    record_runtime_event(EVENT_SERVICE_TASK);
    runtime_observed.ready_during_last_task_create = SamCspRuntime_IsReady();
    if (runtime_observed.fail_stage == FAIL_SERVICE_TASK) {
        return NULL;
    }
    runtime_observed.task_handles[index] = &fake_service_task;
    return &fake_service_task;
}

static void runtime_fake_task_delete(TaskHandle_t task)
{
    vTaskDelete(task);
}

static int runtime_fake_service_prepare(void)
{
    record_runtime_event(EVENT_SERVICE_PREPARE);
    ++runtime_observed.service_prepare_calls;
    switch (runtime_observed.fail_stage) {
        case FAIL_SERVICE_SOCKET:
            return SAM_CSP_RUNTIME_ERR_SERVICE_SOCKET;
        case FAIL_SERVICE_BIND:
            return SAM_CSP_RUNTIME_ERR_SERVICE_BIND;
        case FAIL_SERVICE_LISTEN:
            return SAM_CSP_RUNTIME_ERR_SERVICE_LISTEN;
        default:
            return CSP_ERR_NONE;
    }
}

static void runtime_fake_force_receive(void)
{
    ++runtime_observed.force_receive_calls;
}

static void runtime_fake_report_failure(int init_code)
{
    ++runtime_observed.report_failure_calls;
    runtime_observed.reported_failure = init_code;
}

static const sam_csp_runtime_ops_t fake_runtime_ops = {
    .csp_init = runtime_fake_csp_init,
    .link_init = runtime_fake_link_init,
    .link_deinit = runtime_fake_link_deinit,
    .link_get_interface = runtime_fake_get_interface,
    .route_set = runtime_fake_route_set,
    .task_create_static = runtime_fake_task_create,
    .task_delete = runtime_fake_task_delete,
    .service_prepare = runtime_fake_service_prepare,
    .link_force_receive = runtime_fake_force_receive,
    .report_failure = runtime_fake_report_failure,
};

static void reset_runtime_fakes(runtime_fail_stage_t fail_stage)
{
    fake_freertos_reset();
    memset(&runtime_observed, 0, sizeof(runtime_observed));
    memset(&fake_router_task, 0, sizeof(fake_router_task));
    memset(&fake_service_task, 0, sizeof(fake_service_task));
    runtime_observed.fail_stage = fail_stage;
    sam_csp_runtime_test_reset();
    sam_csp_runtime_test_bind_ops(&fake_runtime_ops);
}

static void runtime_applies_exact_configuration_and_startup_order(void)
{
    static const int expected_events[] = {
        EVENT_CSP_INIT,
        EVENT_LINK_INIT,
        EVENT_INTERFACE,
        EVENT_ROUTE,
        EVENT_ROUTER_TASK,
        EVENT_SERVICE_PREPARE,
        EVENT_SERVICE_TASK,
    };
    sam_csp_runtime_status_t status;

    reset_runtime_fakes(FAIL_NONE);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RUNTIME_OK, SamCspRuntime_Init());
    TEST_ASSERT_TRUE(SamCspRuntime_IsReady());
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RUNTIME_OK, SamCspRuntime_GetInitCode());
    TEST_ASSERT_TRUE(!runtime_observed.ready_during_last_task_create);
    TEST_ASSERT_EQ_SIZE(
        sizeof(expected_events) / sizeof(expected_events[0]),
        runtime_observed.event_count);
    for (size_t index = 0U;
         index < (sizeof(expected_events) / sizeof(expected_events[0]));
         ++index) {
        TEST_ASSERT_EQ_SIZE(
            (size_t) expected_events[index],
            (size_t) runtime_observed.events[index]);
    }
    TEST_ASSERT_EQ_SIZE(SAM_CSP_LOCAL_ADDRESS, runtime_observed.csp_configuration.address);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_CONN_MAX, runtime_observed.csp_configuration.conn_max);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_CONN_QUEUE_LENGTH, runtime_observed.csp_configuration.conn_queue_length);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_FIFO_LENGTH, runtime_observed.csp_configuration.fifo_length);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_MAX_BIND_PORT, runtime_observed.csp_configuration.port_max_bind);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_BUFFER_COUNT, runtime_observed.csp_configuration.buffers);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_BUFFER_DATA_SIZE, runtime_observed.csp_configuration.buffer_data_size);
    TEST_ASSERT_EQ_SIZE(CSP_O_NONE, runtime_observed.csp_configuration.conn_dfl_so);
    TEST_ASSERT_TRUE(runtime_observed.link_configuration.port_ops == &samv71_rs485_port_ops);
    TEST_ASSERT_TRUE(runtime_observed.link_configuration.port_context == &samv71_rs485_port_context);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_TX_MARGIN_MS, runtime_observed.link_configuration.tx_margin_ms);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RECOVERY_RETRY_MS, runtime_observed.link_configuration.recovery_retry_ms);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_LINK_TASK_PRIORITY, runtime_observed.link_configuration.task_priority);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_LINK_TASK_STACK_WORDS, runtime_observed.link_configuration.task_stack_words);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_PEER_ADDRESS, runtime_observed.route_destination);
    TEST_ASSERT_TRUE(runtime_observed.route_interface == (csp_iface_t *) (void *) &fake_interface_object);
    TEST_ASSERT_EQ_SIZE(CSP_NO_VIA_ADDRESS, runtime_observed.route_via);
    TEST_ASSERT_EQ_SIZE(2U, runtime_observed.task_create_calls);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_ROUTER_TASK_STACK_WORDS, runtime_observed.task_stacks[0]);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_ROUTER_TASK_PRIORITY, runtime_observed.task_priorities[0]);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_SERVICE_TASK_STACK_WORDS, runtime_observed.task_stacks[1]);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_SERVICE_TASK_PRIORITY, runtime_observed.task_priorities[1]);
    TEST_ASSERT_TRUE(strcmp("csp-router", runtime_observed.task_names[0]) == 0);
    TEST_ASSERT_TRUE(strcmp("csp-service", runtime_observed.task_names[1]) == 0);
    SamCspRuntime_GetStatus(&status);
    TEST_ASSERT_TRUE(status.ready);
    TEST_ASSERT_TRUE(status.router_task == &fake_router_task);
    TEST_ASSERT_TRUE(status.service_task == &fake_service_task);
}

static void runtime_is_one_shot_after_success(void)
{
    reset_runtime_fakes(FAIL_NONE);
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RUNTIME_OK, SamCspRuntime_Init());
    TEST_ASSERT_EQ_SIZE(SAM_CSP_RUNTIME_OK, SamCspRuntime_Init());
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.csp_init_calls);
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.link_init_calls);
    TEST_ASSERT_EQ_SIZE(2U, runtime_observed.task_create_calls);
}

static void runtime_failure_codes_are_stable_and_cleanup_is_stage_aware(void)
{
    static const struct {
        runtime_fail_stage_t stage;
        int expected_code;
        uint8_t link_was_established;
        uint8_t router_was_created;
    } cases[] = {
        {FAIL_CSP_INIT, SAM_CSP_RUNTIME_ERR_CSP_INIT, 0U, 0U},
        {FAIL_LINK_INIT, SAM_CSP_RUNTIME_ERR_LINK_INIT, 0U, 0U},
        {FAIL_INTERFACE, SAM_CSP_RUNTIME_ERR_INTERFACE, 1U, 0U},
        {FAIL_ROUTE, SAM_CSP_RUNTIME_ERR_ROUTE, 1U, 0U},
        {FAIL_ROUTER_TASK, SAM_CSP_RUNTIME_ERR_ROUTER_TASK, 1U, 0U},
        {FAIL_SERVICE_SOCKET, SAM_CSP_RUNTIME_ERR_SERVICE_SOCKET, 1U, 1U},
        {FAIL_SERVICE_BIND, SAM_CSP_RUNTIME_ERR_SERVICE_BIND, 1U, 1U},
        {FAIL_SERVICE_LISTEN, SAM_CSP_RUNTIME_ERR_SERVICE_LISTEN, 1U, 1U},
        {FAIL_SERVICE_TASK, SAM_CSP_RUNTIME_ERR_SERVICE_TASK, 1U, 1U},
    };

    for (size_t index = 0U; index < (sizeof(cases) / sizeof(cases[0])); ++index) {
        reset_runtime_fakes(cases[index].stage);
        TEST_ASSERT_EQ_SIZE(
            (size_t) cases[index].expected_code,
            (size_t) SamCspRuntime_Init());
        TEST_ASSERT_TRUE(!SamCspRuntime_IsReady());
        TEST_ASSERT_EQ_SIZE(
            (size_t) cases[index].expected_code,
            (size_t) SamCspRuntime_GetInitCode());
        TEST_ASSERT_EQ_SIZE(1U, runtime_observed.force_receive_calls);
        TEST_ASSERT_EQ_SIZE(
            cases[index].link_was_established,
            runtime_observed.link_deinit_calls);
        TEST_ASSERT_EQ_SIZE(1U, runtime_observed.report_failure_calls);
        TEST_ASSERT_EQ_SIZE(
            (size_t) cases[index].expected_code,
            (size_t) runtime_observed.reported_failure);
        fake_freertos_observations_t freertos_observed;
        fake_freertos_get_observations(&freertos_observed);
        TEST_ASSERT_EQ_SIZE(
            cases[index].router_was_created,
            freertos_observed.non_null_delete_calls);
        sam_csp_runtime_status_t status;
        SamCspRuntime_GetStatus(&status);
        TEST_ASSERT_TRUE(status.router_task == NULL);
        TEST_ASSERT_TRUE(status.service_task == NULL);
    }
}

static void runtime_is_one_shot_after_failure(void)
{
    reset_runtime_fakes(FAIL_ROUTE);
    TEST_ASSERT_EQ_SIZE(
        (size_t) SAM_CSP_RUNTIME_ERR_ROUTE,
        (size_t) SamCspRuntime_Init());
    TEST_ASSERT_EQ_SIZE(
        (size_t) SAM_CSP_RUNTIME_ERR_ROUTE,
        (size_t) SamCspRuntime_Init());
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.csp_init_calls);
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.link_init_calls);
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.route_calls);
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.link_deinit_calls);
    TEST_ASSERT_EQ_SIZE(1U, runtime_observed.force_receive_calls);
}

const test_case_t sam_csp_runtime_tests[] = {
    {"sam_csp_runtime", "exact config and startup order publish ready last", runtime_applies_exact_configuration_and_startup_order},
    {"sam_csp_runtime", "successful initialization is one shot", runtime_is_one_shot_after_success},
    {"sam_csp_runtime", "failure codes and cleanup are stage aware", runtime_failure_codes_are_stable_and_cleanup_is_stage_aware},
    {"sam_csp_runtime", "failed initialization is one shot", runtime_is_one_shot_after_failure},
};

const size_t sam_csp_runtime_test_count =
    sizeof(sam_csp_runtime_tests) / sizeof(sam_csp_runtime_tests[0]);
