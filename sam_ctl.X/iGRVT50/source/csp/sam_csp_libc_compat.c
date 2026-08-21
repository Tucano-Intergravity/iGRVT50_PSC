#include <stddef.h>
#include <stdint.h>

#include <csp/arch/csp_system.h>

#include "FreeRTOS.h"

static int lower_ascii(int value)
{
    if ((value >= 'A') && (value <= 'Z')) {
        return value + ('a' - 'A');
    }
    return value;
}

int SamCsp_Strncasecmp(const char *left, const char *right, size_t count)
{
    size_t i;

    if (count == 0U) {
        return 0;
    }
    if (left == right) {
        return 0;
    }
    if (left == NULL) {
        return -1;
    }
    if (right == NULL) {
        return 1;
    }

    for (i = 0U; i < count; i++) {
        const int lc = lower_ascii((unsigned char)left[i]);
        const int rc = lower_ascii((unsigned char)right[i]);
        if ((lc != rc) || (lc == '\0')) {
            return lc - rc;
        }
    }
    return 0;
}

int SamCsp_Strcasecmp(const char *left, const char *right)
{
    if (left == right) {
        return 0;
    }
    if (left == NULL) {
        return -1;
    }
    if (right == NULL) {
        return 1;
    }

    for (;;) {
        const int lc = lower_ascii((unsigned char)*left);
        const int rc = lower_ascii((unsigned char)*right);
        if ((lc != rc) || (lc == '\0')) {
            return lc - rc;
        }
        left++;
        right++;
    }
}

int csp_sys_tasklist(char *out)
{
    if (out != NULL) {
        out[0] = '\0';
    }
    return 0;
}

int csp_sys_tasklist_size(void)
{
    return 1;
}

uint32_t csp_sys_memfree(void)
{
    return (uint32_t)xPortGetFreeHeapSize();
}

void csp_sys_set_color(csp_color_t color)
{
    (void)color;
}
