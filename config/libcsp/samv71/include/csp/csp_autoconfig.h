#ifndef CSP_AUTOCONFIG_H
#define CSP_AUTOCONFIG_H

#include <stddef.h>

/* SAMV71 target profile for the pinned libcsp v1.6 revision. */

/* XC32 exposes this POSIX-compatible builtin without shipping its symbol. */
int strncasecmp(const char *left, const char *right, size_t count);

#define GIT_REV "v1.6-0-g87006959"
#define LIBCSP_VERSION "1.6"

#define CSP_FREERTOS 1
#define CSP_POSIX 0
#define CSP_WINDOWS 0
#define CSP_MACOSX 0

#define CSP_DEBUG 1
#define CSP_DEBUG_TIMESTAMP 0
#define CSP_USE_ASSERT 1

#define CSP_USE_RDP 0
#define CSP_USE_RDP_FAST_CLOSE 0
#define CSP_USE_CRC32 1
#define CSP_USE_HMAC 0
#define CSP_USE_XTEA 0
#define CSP_USE_PROMISC 0
#define CSP_USE_QOS 0
#define CSP_USE_DEDUP 0
#define CSP_USE_EXTERNAL_DEBUG 0

#define CSP_LOG_LEVEL_DEBUG 0
#define CSP_LOG_LEVEL_INFO 0
#define CSP_LOG_LEVEL_WARN 0
#define CSP_LOG_LEVEL_ERROR 1

#define CSP_LITTLE_ENDIAN 1
#define CSP_BIG_ENDIAN 0

#define CSP_HAVE_LIBSOCKETCAN 0
#define CSP_HAVE_LIBZMQ 0

#endif
