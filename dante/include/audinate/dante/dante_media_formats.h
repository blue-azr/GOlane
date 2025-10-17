/*
 * File     : dante_media_formats.h
 * Created  : May 2019
 * Author   : Andrew White
 * Synopsis : Unified dante format handling
 *
 * This software is copyright (c) 2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1 
 */

/**
 * @file dante_media_formats.h
 * Definition of a polymorphic Dante format type
 */

#ifndef _DANTE_MEDIA_FORMATS_H
#define _DANTE_MEDIA_FORMATS_H

//----------------------------------------------------------
// Include
//----------------------------------------------------------

#include "dante_media.h"
#include "dante_video.h"
#include "dante_ancillary.h"

#ifdef __cplusplus
extern "C" {
#endif


//----------------------------------------------------------
// Dante media formats
//----------------------------------------------------------

typedef struct dante_media_format dante_media_format_t;


// Specific format accessors

const dante_format_t *
dante_audio_format_from_media_format(const dante_media_format_t *);

const dante_formats_t *
dante_audio_formats_from_media_format(const dante_media_format_t *);

const dante_video_format_t *
dante_video_format_from_media_format(const dante_media_format_t *);

const dante_ancillary_format_t *
dante_ancillary_format_from_media_format(const dante_media_format_t *);


// Queries

dante_media_type_t
dante_media_format_type(const dante_media_format_t *);


// Comparison

aud_bool_t
dante_media_format_compatible
(
	const dante_media_format_t *,
	const dante_media_format_t *
);

aud_bool_t
dante_media_format_equals
(
	const dante_media_format_t *,
	const dante_media_format_t *
);


//----------------------------------------------------------
// Dante media format list

/**
	A list of Dante media formats.

	Currently, media format lists are used by some video devices to describe the various codecs that
	a given channel can support.  The list contains zero or more Dante media formats in an array.

	Unlike dante_media_format_t, a media format list is dynamically allocated and must be managed with
	clone and free operations.

	Unless otherwise specified, the order of elements is arbitrary.
 */
typedef struct dante_media_format_list dante_media_format_list_t;


// Memory operations

/*
	Clone a media format list.  The resulting list is an exact copy at the
	point of cloning.  The caller is responsible for freeing the clone when done.
 */
dante_media_format_list_t *
dante_media_format_list__clone
(
	dante_media_format_list_t * list
);

/*
	Deallocate a cloned dante_media_format_list.

	@note This function should only be called on lists cloned by the caller.
 */
void
dante_media_format_list__free
(
	dante_media_format_list_t * list
);


// Examining elements

/**
	Number of elements in list
 */
size_t
dante_media_format_list__length
(
	const dante_media_format_list_t *
);

size_t
dante_media_format_list__max_length
(
	const dante_media_format_list_t * list
);


/**
	Access a particular element in the list.

	Format data structure returned is part of the list, and will go out of scope if the list goes out of scope.

	Returns NULL on out-of-range access.  It is legitimate to iterate through the list until a NULL is returned.
 */
const dante_media_format_t *
dante_media_format_list__at_index
(
	const dante_media_format_list_t *,
	unsigned idx
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif

