/*
* File     : dante_domains.h
* Created  : 23 Aug 2016 12:19:03
* Author   : James Westendorp
* Synopsis : Dante domains types and structures
*
* This software is copyright (c) 2004-2022 Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1
*/

/**
 * @file dante_domains.h
 * Dante Domain types and structures
 */

#ifndef _DANTE_DOMAINS_H
#define _DANTE_DOMAINS_H


#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#endif

#define DANTE_DOMAIN_NAME_LENGTH 128
#define DANTE_DOMAIN_UUID_LENGTH 16

#ifdef __cplusplus
#define DANTE_DOMAIN_UUID_CAST
#else
#define DANTE_DOMAIN_UUID_CAST (dante_domain_uuid_t)
#endif

#define DANTE_DOMAIN_UUID_ADHOC_BYTES {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}

#define DANTE_DOMAIN_UUID_1			DANTE_DOMAIN_UUID_CAST{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01} }
#define DANTE_DOMAIN_UUID_NONE		DANTE_DOMAIN_UUID_CAST{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} }
#define DANTE_DOMAIN_UUID_ADHOC 	DANTE_DOMAIN_UUID_CAST{ DANTE_DOMAIN_UUID_ADHOC_BYTES }
#define DANTE_DOMAIN_UUID_UNKNOWN	DANTE_DOMAIN_UUID_CAST{ DANTE_DOMAIN_UUID_ADHOC_BYTES }
	// Used when 'ADHOC' is not a valid identifier for an unknown UUID. e.g. Peer Group ID

#define IS_NO_DOMAIN_UUID(X)    	(dante_domain_uuid_cmp(X, DANTE_DOMAIN_UUID_NONE) == 0)
#define IS_ADHOC_DOMAIN_UUID(X)		(dante_domain_uuid_cmp(X, DANTE_DOMAIN_UUID_ADHOC) == 0)
#define IS_MANAGED_DOMAIN_UUID(X)   (!IS_NO_DOMAIN_UUID(X) && !IS_ADHOC_DOMAIN_UUID(X))
#define IS_DDM_DOMAIN_UUID(X)   	(IS_MANAGED_DOMAIN_UUID(X))

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t dante_domain_id_t;
#define DANTE_DOMAIN_ID_NONE 0x0000
#define DANTE_DOMAIN_ID_ADHOC 0xFFFF

union dante_domain_uuid
{
	uint8_t data[16];
	uint32_t data32[4];
};

// 128 bit uuid
typedef union dante_domain_uuid dante_domain_uuid_t;

/*
	return 0 if the ids are equal, non-zero otherwise
*/
int32_t dante_domain_uuid_cmp(dante_domain_uuid_t a, dante_domain_uuid_t b);


/**
 * Every endpoint (device or controller) in a Dante domain has a unique identifier for
 * The purposes of packet routing. Each endpoint may consist of multiple components.
 * The <endpoint, component> tuple provides a unique identifier for packet routing
 * within a Dante domain.
 */
typedef uint32_t dante_domain_endpoint_id_t;
typedef uint16_t dante_domain_component_id_t;

typedef struct dante_domain_routing_id
{
	dante_domain_endpoint_id_t endpoint_id;
	dante_domain_component_id_t component_id;
} dante_domain_routing_id_t;


aud_bool_t dante_domain_handler_is_pgid_none(dante_domain_uuid_t uuid);
aud_bool_t dante_domain_handler_is_pgid_unknown(dante_domain_uuid_t uuid);


// Strings

#define DANTE_DOMAIN_UUID_HEX_LENGTH 33
#define DANTE_DOMAIN_UUID_STRING_LEN 37

struct dante_domain_uuid_string
{
	char str[DANTE_DOMAIN_UUID_STRING_LEN];
};

typedef struct dante_domain_uuid_string dante_domain_uuid_string_t;

struct dante_domain_uuid_hex
{
	char str[DANTE_DOMAIN_UUID_HEX_LENGTH];
};

typedef struct dante_domain_uuid_hex dante_domain_uuid_hex_t;

/**
 * Render a domain UUID as a 32 character string.
 *
 * Output format is 32 lowercase hex digits followed by a nul.
 *
 * @param[in] id_bytes Dante domain UUID to render
 * @param[out] id_hex Buffer to hold rendered string
 */
aud_error_t
dante_domain_uuid_to_hex
(
	const dante_domain_uuid_t* id_bytes,
	dante_domain_uuid_hex_t* id_hex
);

/**
 * Convert a string of 32 hex characters (0-9, a-f, A-F) into a domain UUID
 *
 * @param[in] id_hex string representation of Domain UUID.
 *	Must contain only 32 valid hex characters terminated by a nul.
 * @param[out] id_bytes output UUID.
 *	Must be valid pointer.  Set to zero on failure.
 *
 */
aud_error_t
dante_domain_uuid_from_hex
(
	const char* id_hex,
	dante_domain_uuid_t* id_bytes
);

/**
 * Render a domain UUID as a structured string.
 *
 * Output format is 36 characters formatted as `01234567-89ab-cdef-0123-456789abcdef`, followed by nul.
 *
 * @param[in] id_bytes Dante domain UUID to render
 * @param[out] id_hex Buffer to hold rendered string
 */

aud_error_t
dante_domain_uuid_to_string
(
	const dante_domain_uuid_t* id_bytes,
	dante_domain_uuid_string_t* id_string
);

/**
 * Convert a structured string (as rendered by dante_domain_uuid_to_string() ) into a domain UUID
 *
 * @param[in] id_hex string representation of Domain UUID.
 *	Must contain exactly 36 valid hex characters and dashes terminated by a nul.
 * @param[out] id_bytes output UUID.
 *	Must be valid pointer.  Set to zero on failure.
 *
 */
aud_error_t
dante_domain_uuid_from_string
(
	const char* id_string,
	dante_domain_uuid_t* id_bytes
);


// Parsing Dante Domain IDs

/**
 * Parse a domain UUID from the head of a string.
 *
 * This function is provided to make it easy to pull a Domain UUID out of a string entered by a user.
 * For example, via text entry in sample code.  It provides only loose verification.
 * String conversions for machine use should use dante_domain_uuid_to_hex() and dante_domain_uuid_from_hex()
 *
 * @param[in] id_hex string representation of Domain UUID.
 *	See below for format
 * @param[out] id_bytes output UUID.
 *	Must be valid pointer.  On failure, contains parsed portion and with remainder set 0.
 * @param[out] next if non-NULL, points to first character not parsed into UUID
 *
 * String parsing rules:
 * - Length
 *  - String may not contain more than 32 hex digits.  Exceeding this causes an error.
 *  - String may contain fewer than 32 hex digits.  Tail is padded to zero.
 * - Whitespace
 *  - Leading whitespace is skipped
 *  - Other whitespace will terminate parse
 * - Non-hex letters
 *  - String may not contain embedded letters that are not legal hex digits.
 *   Error if isalnum(ch) && ! isxdigit(ch)
 * - Other characters
 *   - String may contain embedded ':' or '-' (will be ignored).
 *   - Parsing will terminate at ','
 *   - Any other non-digit character will cause an error.
 *   - Treatment of punctuation characters may be relaxed in future releases.
 * - Parsing terminates successfully at first trailing whitespace, end-of-string, or ','.
 */
aud_error_t
dante_domain_uuid_parse
(
	const char* id_string,
	dante_domain_uuid_t* id_bytes,
	char ** next
);



#ifdef __cplusplus
}
#endif

#endif /* _DANTE_DOMAINS_H */
