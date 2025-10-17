/*
 * File     : dante_video.h
 * Created  : May 2019
 * Author   : Andrew White
 * Synopsis : Common Dante video types
 *
 * This software is copyright (c) 2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

/**
 * @file dante_video.h
 * Types and structures for Dante Video
 */

#ifndef _DANTE_VIDEO_H
#define _DANTE_VIDEO_H

//----------------------------------------------------------
// Include
//----------------------------------------------------------

#include "dante_common.h"

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// Dante video formats
//----------------------------------------------------------

/**
 * Identifier for a video format within DAPI
 *
 * From a DAPI / routing perspective, Dante treats video formats as opaque
 * identifiers.
 */
typedef union dante_video_format dante_video_format_t;


// Video format operations

aud_bool_t
dante_video_format_equals
(
	const dante_video_format_t *,
	const dante_video_format_t *
);

void
dante_video_format_copy
(
	dante_video_format_t * dst,
	const dante_video_format_t * src
);

aud_bool_t
dante_video_format_is_valid
(
	const dante_video_format_t *
);

void
dante_video_format_init_invalid
(
	dante_video_format_t *
);


/**
 * Test if two codec formats are identical
 *
 * @param[in] f1 video format
 * @param[in] f2 video format
 *
 * @return true if video codec values identical between formats
 */
aud_bool_t
dante_video_format__codec_equal
(
	const dante_video_format_t * f1,
	const dante_video_format_t * f2
);


/**
 * Check codec compatibility
 *
 * @param[in] f1 video format
 * @param[in] f2 video format
 *
 * @return true if video codec values are compatible between formats
 *
 * @note This is not definitive.
 *	- There may be video profile implementation details that cause otherwise
 *    compatible formats to be incompatible at runtime.
 *	- If insufficient information is available to compare, will return false.
 */
aud_bool_t
dante_video_format__codec_compatible
(
	const dante_video_format_t * f1,
	const dante_video_format_t * f2
);


// Memory management

/**
 * Buffer to hold a dante_video_format
 *
 * This buffer has the memory required to store a dante video format.
 *
 * \warning The size of this structure may change between DAPI releases.
 *	Always use sizeof() if the size needs to be known.
 */
typedef struct dante_video_format_buffer
{
	uint32_t data[4];
} dante_video_format_buffer_t;


/**
 * Treat a dante_video_format buffer as a video format.
 *
 * Use this function rather than direct access to protect against API change.
 */
AUD_INLINE dante_video_format_t *
dante_video_format_from_buffer
(
	dante_video_format_buffer_t * buffer
)
{
	return (dante_video_format_t *) buffer;
}


/**
 * Treat a dante_video_format buffer as a video format.
 *
 * Const version of dante_video_format_from_buffer().
 */
AUD_INLINE const dante_video_format_t *
dante_video_format_from_buffer_const
(
	const dante_video_format_buffer_t * buffer
)
{
	return (const dante_video_format_t *) buffer;
}


// String utilities

/**
	Render a dante video format as a string

	@param[in] f video format
	@param[out] str destination string buffer
	@param[in] len length of buffer
 */
size_t
dante_video_format_to_string
(
	const dante_video_format_t * f,
	char * str,
	size_t len
);


//-------------------------------------------
// Video subtype
//-------------------------------------------

/**
	RTP video subtypes defined within Dante

	Video subtypes usually correspond to "media subtypes" registered under RFC4855.
	A media subtype is colloquially a "codec type" or "codec family".
 */
typedef enum dante_video_subtype
{
	DANTE_VIDEO_SUBTYPE__UNDEF = 0,

	// Values 1-127 are reserved to avoid confusion with RFC3551 registrations

	DANTE_VIDEO_SUBTYPE__JPEG2000 = 129,
	DANTE_VIDEO_SUBTYPE__H264 = 130,
	DANTE_VIDEO_SUBTYPE__BLUERIVER = 131,
	DANTE_VIDEO_SUBTYPE__COLIBRI = 132,
	DANTE_VIDEO_SUBTYPE__ASPEED_AV_A = 133,
	DANTE_VIDEO_SUBTYPE__H265 = 134,
	DANTE_VIDEO_SUBTYPE__HTJ2K = 135,

} dante_video_subtype_t;


// Operations

/**
	Video subtype of a video format

	@returns video subtype of video format,
		or DANTE_VIDEO_SUBTYPE__UNDEF if format is NULL.
 */
dante_video_subtype_t
dante_video_format__subtype(const dante_video_format_t * format);


// String handling

/*
	String representations supported by Dante utilities

	Dante utilities will render strings using the canonical form, but from_string
	utilities will also accept the alternative forms.  from_string utilities are
	case-insensitive.
 */


#define DANTE_VIDEO_SUBTYPE_STRING__JPEG2000 "JPEG2000"
#define DANTE_VIDEO_SUBTYPE_STRING__JPEG2000__J2K "J2K"

#define DANTE_VIDEO_SUBTYPE_STRING__H264 "H.264/AVC"
#define DANTE_VIDEO_SUBTYPE_STRING__H264__SHORT "H264"

#define DANTE_VIDEO_SUBTYPE_STRING__BLUERIVER "BlueRiver"

#define DANTE_VIDEO_SUBTYPE_STRING__COLIBRI "Colibri"
#define DANTE_VIDEO_SUBTYPE_STRING__ASPEED_AV_A "AV-A"
#define DANTE_VIDEO_SUBTYPE_STRING__H265 "H.265/HEVC"
#define DANTE_VIDEO_SUBTYPE_STRING__H265__SHORT "H265"
#define DANTE_VIDEO_SUBTYPE_STRING__HTJ2K "HTJ2K"


/**
	String representation of dante video subtype value.

	@param[in] s video subtype

	@returns string representation of subtype value
		- "?" on UNDEF (0)
		- NULL on undefined values
 */
const char *
dante_video_subtype_to_string(dante_video_subtype_t s);

/**
	Convert a string to a dante video subtype.

	Parsing terminates at the first character that is not one of [A-Za-z0-9+-],
	unless that character is a punctuation character that is part of the preceeding
	subtype name.  Whitespace or end-of-string will always terminate parsing.

	@param[in] str string representation of video subtype.
	@param[out] if non-NULL, will contain pointer to first unparsed character.

	@returns dante_video_subtype corresponding to string
		- UNDEF on undefined / unparseable values
 */
dante_video_subtype_t
dante_video_subtype_parse_from_string
(
	const char * str,
	char ** next
);

/**
	Convert a string to a dante video subtype.

	Short version of dante_video_subtype_parse_from_string().

	@param[in] str string representation of video subtype.

	@returns dante_video_subtype corresponding to string
		- UNDEF on undefined / unparseable values
 */
AUD_INLINE dante_video_subtype_t
dante_video_subtype_from_string(const char * str)
{
	return dante_video_subtype_parse_from_string(str, NULL);
}


// Video codec ID

typedef uint32_t dante_video_codec_id_t;

dante_video_codec_id_t
dante_video_format__codec_id(const dante_video_format_t * format);


//-------------------------------------------
// HDCP key negotiation status
//-------------------------------------------

/*Enumertaion of the HDCP key negotiation status reported by receiver*/

enum dante_hdcp_nego_status
{
	DANTE_HDCP_NEGO_STATUS__NONE = 0,

	/* RX finding server to obtain key, status between 60 and 69 */
	DANTE_HDCP_NEGO_STATUS__RESOLVING_SOURCE = 60,
	//Rx is resolving the server

	DANTE_HDCP_NEGO_STATUS__AWAITING_RESPONSE = 61,
	//Rx is waiting for a response from server

	/* Negotiation in progress status between 100 and 199 */

	DANTE_HDCP_NEGO_STATUS__NEGOTIATING = 100,
	/*Key negotiation in progress*/

	DANTE_HDCP_NEGO_STATUS__ACQUIRING_PORT = 101,
	/*Acquiring port for key negotiation*/

	/* Negotiation success status between 200 and 299 */

	DANTE_HDCP_NEGO_STATUS__NEGOTIATED = 200,
	/*Key negotiation complete.*/

	/* Negotiation failure status between 300 and 399 */
	DANTE_HDCP_NEGO_STATUS__NEGOTIATION_ERROR = 300,

	/* Negotiation failure: Handshake error status between between 301 and 320 */
	DANTE_HDCP_NEGO_STATUS__NO_TX = 301,
	//Transmitter does not exist
	DANTE_HDCP_NEGO_STATUS__RESOURCE_UNAVAILABLE = 302,
	//resource not available (port already in use)
	DANTE_HDCP_NEGO_STATUS__TX_RESPONSE_INVALID = 303,
	//Transmitter handshake response is invalid
	DANTE_HDCP_NEGO_STATUS__NO_TX_FLOW = 304,
	//no transmit flow configured in TX manager
	DANTE_HDCP_NEGO_STATUS__VERSION_MISMATCH = 305,
	//handshake version mismatch
	DANTE_HDCP_NEGO_STATUS__INVALID_TOKEN = 306,
	//invalid token on handshake

	/* Negotiation failure: Authentication error status between between 321 and 330 */
	DANTE_HDCP_NEGO_STATUS__UNAUTHENTICATED = 321,
	//Unauthenticated
	DANTE_HDCP_NEGO_STATUS__INTERNAL_STACK_ERROR = 322,
	//Internal HDCP stack error
	DANTE_HDCP_NEGO_STATUS__STACK_RESOURCE_ERROR = 323,
	//HDCP Stack configuration error
};

typedef uint16_t dante_hdcp_nego_status_t;

const char *
dante_negotiation_status_to_string(dante_hdcp_nego_status_t status);

//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

