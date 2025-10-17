/*
 * File     : dante_ancilary.h
 * Created  : May 2019
 * Author   : Andrew White <andrew.white@audinate.com>
 * Synopsis : Common Dante ancillary data types
 *
 * This software is copyright (c) 2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file dante_ancillary.h
 * Types and structures for Dante Ancillary data
 */

#ifndef _DANTE_ANCILLARY_H
#define _DANTE_ANCILLARY_H

//----------------------------------------------------------
// Include
//----------------------------------------------------------

#include "dante_common.h"

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// Dante ancillary flow class
//----------------------------------------------------------

typedef uint16_t dante_ancillary_flow_class_t;

enum dante_ancillary_flow_class
{
	DANTE_ANCILLARY_FLOW_CLASS__UNDEF = 0,
	DANTE_ANCILLARY_FLOW_CLASS__USBHID = 1,
	DANTE_ANCILLARY_FLOW_CLASS__IR = 2,
	DANTE_ANCILLARY_FLOW_CLASS__SERIAL = 3,
};


const char *
dante_ancillary_flow_class_to_string
(
	dante_ancillary_flow_class_t flow_class
);

dante_ancillary_flow_class_t
dante_ancillary_flow_class_from_string
(
	const char * flow_class_str
);

//----------------------------------------------------------
// Dante ancillary formats
//----------------------------------------------------------

/**
 * Identifier for an ancillary data format
 */
typedef struct dante_acillary_format
{
	uint32_t type;
} dante_ancillary_format_t;


// Ancillary format operations

aud_bool_t
dante_ancillary_format_equals
(
	const dante_ancillary_format_t *,
	const dante_ancillary_format_t *
);

void
dante_ancillary_format_copy
(
	dante_ancillary_format_t * dst,
	const dante_ancillary_format_t * src
);

aud_bool_t
dante_ancillary_format_is_valid
(
	const dante_ancillary_format_t *
);

void
dante_ancillary_format_init_invalid
(
	dante_ancillary_format_t *
);


// Utilities

size_t
dante_ancillary_format_to_string
(
	const dante_ancillary_format_t *,
	char * str,
	size_t len
);

/**
 * Read an ancillary format from a string
 *
 * @param str source string
 * @param f destination ancillary format
 * @param next if non-null, on return will point to first unparsed character
 *    (character after the string on success, character that broke parsing on fail)
 */
aud_error_t
dante_ancillary_format_from_string_prefix
(
	const char * str,
	dante_ancillary_format_t * f,
	char ** next
);

AUD_INLINE aud_error_t
dante_ancillary_format_from_string
(
	const char * str,
	dante_ancillary_format_t * f
)
{
	return dante_ancillary_format_from_string_prefix(str, f, NULL);
}



//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

