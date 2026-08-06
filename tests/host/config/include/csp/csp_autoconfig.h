/* Host profile derived from the authorized C:\PSC\csp-rs485 test configuration. */
/* This import makes no assertion about upstream licensing. */
#ifndef CSP_AUTOCONFIG_H
#define CSP_AUTOCONFIG_H

/* POSIX-only configuration for the local CSP RS485 host regression gate. */
#define GIT_REV "v1.6-0-g87006959"
#define LIBCSP_VERSION "1.6"

#define CSP_FREERTOS 0
#define CSP_POSIX 1
#define CSP_WINDOWS 0
#define CSP_MACOSX 0

#define CSP_DEBUG 1
#define CSP_DEBUG_TIMESTAMP 0
#define CSP_USE_ASSERT 1

#define CSP_USE_RDP 1
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
