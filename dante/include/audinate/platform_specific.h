/*
 * File     : $RCSfile$
 * Created  : March 2007
 * Updated  : $Date$
 * Author   : James Westendorp
 * Synopsis : Linux implementation of the Audinate Cross-Platform API
 *
 * This software is copyright (c) 2004-2016 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#ifndef _PLATFORM_SPECIFIC_H
#define _PLATFORM_SPECIFIC_H

// needed for strdup & others when
// compiling with -ansi or similar
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// needed for struct timeval
#include <sys/time.h>

// needed for syslog
#include <sys/syslog.h>

// needed for struct in_addr
#include <netinet/in.h>

// needed for inet_ntoa
#include <arpa/inet.h>

// needed for wchar_t
#include <wchar.h>

#define AUD_PLATFORM "armv7"

#ifdef __cplusplus
extern "C" {
#endif

typedef float float32_t;

typedef struct timeval aud_utime_t;

typedef int aud_socket_t;

/**
 * Ubuntu 6.06: a timeval is __time_t/ __suseconds_t = long int / long int
 * This is the format used by ubuntu
 * if other linux variants need other types, we'll need to enclose these
 * definitions in a #ifdef of some kind.
 */
typedef long aud_tv_usec_t;

#define AUD_INLINE static __inline__
enum 
{
	AUD_ENV_ERROR_STRLEN = 512
};

#define AUD_SOCKET_INVALID -1

#define AUD_ENV_HAS_SELECT 1

/** Linux supports interrupt signals */
#define AUD_ENV_CAN_SIGINT 1

/** Linux supports the conmon manager API */
#define AUD_ENV_HAS_CMM 1

/** Linux does not have a conmon metering core */
#define AUD_ENV_HAS_CONMON_METERING_CORE 0

/** no extra functions */
#define AUD_ENV_HAS_PLATFORM_EXTRA 0

#define AUD_ENV_HAS_STDIN 1
#define AUD_ENV_HAS_SELECTABLE_STDIN 1


/**
 * What is the minimum version of Bonjour needed,
 * defined in dns_sd.h as major*10000 + minor*100
 * Currently set to version 2.0 ie. 200*10000 + 0*100
 */
#define AUD_ENV_MIN_DNSSD_VERSION (200*10000)

#define AUD_FD_COPY(FROM,TO) memcpy(TO,FROM,sizeof(fd_set))

#ifdef __cplusplus
}
#endif

#endif

