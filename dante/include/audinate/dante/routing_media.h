/*
 * File     : dante/routing_media.h
 * Created  : April 2019
 * Author   : Naeem BACHA
 * Synopsis : Media aware Dante Routing Configuration API
 *
 * This software is copyright (c) 2008-2019 Audinate Pty Ltd and/or its licensors
 *
 * Audinate Copyright Header Version 1
 */

#ifndef _DANTE_ROUTING_MEDIA_H
#define _DANTE_ROUTING_MEDIA_H


#ifndef DAPI_FLAT_INCLUDE
#include "dante/dante_media.h"
#include "dante/routing.h"
#include "dante/dante_media_formats.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Media awareness
//----------------------------------------------------------

/**
 * This call enables routing api to become Dante media types aware.
 * Without this call, the routing API only knows about Dante media type = DANTE_MEDIA__AUDIO.
 * Once enabled, the current API instance will remain media aware.
 *
 * @param devices [IN] the devices object
 */
void
dr_devices_set_media_aware
(
	dr_devices_t * devices
);

/**
 * Does the device support multiple media?
 *
 * Return AUD_TRUE if the device can support media other than audio.
 * Return AUD_FALSE if otherwise.
 *
 * @param device [IN] the device object
 */
aud_bool_t
dr_device_is_media_supported
(
	const dr_device_t * device
);

/**
 * Is the media type specified supported by the device?
 *
 * Return AUD_TRUE if the device can support given media type.
 * Return AUD_FALSE if otherwise.
 *
 * @param device [IN] the device object
 * @param mtype  [IN] the media type being checked for support
 */
aud_bool_t
dr_device_is_media_type_supported
(
	const dr_device_t * device,
	dante_media_type_t mtype
);

/**
 * Media sub-device supporting a media type
 */
typedef struct dr_media_device dr_media_device_t;

/**
 * Get the media sub-device for a media type.
 * This is a reference into the dr_device and has exactly the same scoping rules
 *
 * @param device     [IN] the device object
 * @param media_type [IN] the media type of the media sub-device to be returned
 *
 * @note A non-null return means that the media type is known to the routing API,
 *       not that the device particularly supports that media type.
 */
dr_media_device_t *
dr_media_device_for_media_type
(
	dr_device_t * device,
	dante_media_type_t media_type
);

/**
 * Get the main device object for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 * @param device       [OUT] the device object
 */
dr_device_t *
dr_media_device_get_device
(
	const dr_media_device_t * media_device
);

/**
 * Get the media type of a media sub-device object.
 *
 * @param media_device [IN] the media sub-device object
 */
dante_media_type_t
dr_media_device_get_media_type
(
	const dr_media_device_t * media_device
);


//----------------------------------------------------------
// Device state change handlers for media
//----------------------------------------------------------

/**
 * A callback that the host application may use to be notified when media
 * elements of the device have changed.
 *
 * @param device         [IN] the device object
 * @param media_type     [IN] the media type (see note) of the changed device element
 * @param change_flags   [IN] flag that indicates what elements of the device changed
 *
 * @note media_type legal values are:
 *       'UNDEF'    Used for changes not tied to a particular media sub-device
 *       'ALL'      Used for changes tied to all media sub-devices
 *       <SPECIFIC> Used for changes tied to components of a particular media sub-device
 */
typedef void
dr_device_media_changed_fn
(
	dr_device_t * device,
	dante_media_type_t media_type,
	dr_device_change_flags_t change_flags
);

/**
 * Set the device change media callback for this function.
 *
 * @param device         [IN] the device object
 * @param device_changed [IN] the new callback for when the device object changes
 */
void
dr_device_set_media_changed_callback
(
	dr_device_t * device,
	dr_device_media_changed_fn * device_changed
);


//----------------------------------------------------------
// Media device components
//----------------------------------------------------------

/**
 * Is any of device's information for the given component 'stale', ie.
 * needing updating?
 *
 * @param media_device [IN] the media sub-device object
 * @param component    [IN] the component to be checked
 */
aud_bool_t
dr_media_device_is_component_stale
(
	const dr_media_device_t * media_device,
	const dr_device_component_t component
);

/**
 * Forcefully mark an entire media component of a media sub-device as stale.
 * Subsequent component updates will refresh the entire component state.
 *
 * @param media_device [IN] the media sub-device object
 * @param component    [IN] the component to be marked as stale
 *
 * @note Flows and properties will be marked stale for the entire device.
 */
void
dr_media_device_mark_component_stale
(
	const dr_media_device_t * media_device,
	const dr_device_component_t component
);

/**
 * Forcefully mark all media components of a media sub-device as stale.
 * Subsequent components updates will refresh all components' state.
 *
 * @param media_device [IN] the media sub-device object
 *
 * @note Flows and properties will be marked stale for the entire device.
 */
void
dr_media_device_mark_all_components_stale
(
	const dr_media_device_t * media_device
);

/**
 * As dr_device_mark_component_stale() for a particular media device
 *
 * @param device       [IN] the device object
 * @param media_type   [IN] the media type (or DANTE_MEDIA__ALL for all types)
 * @param component    [IN] the component to be marked as stale
 */
void
dr_device_mark_media_component_stale
(
	dr_device_t * device,
	const dante_media_type_t media_type,
	dr_device_component_t component
);


//----------------------------------------------------------
// Media sub-device properties
//----------------------------------------------------------

/**
 * Get a media sub-device's current TX audio delay
 *
 * @param media_device [IN] the media sub-device object
 * @param delay        [IN] pointer that holds the delay value
 *
 * @note if the sub-device is non-audio, AUD_ERR_NOTSUPPORTED will be returned.
 */
aud_error_t
dr_media_device_get_tx_audio_delay
(
	const dr_media_device_t * media_device,
	dante_latency_us_t * delay
);

/**
 * Get a media sub-device's max TX audio delay
 *
 * @param media_device [IN] the media sub-device object
 * @param max_delay    [IN] pointer that holds the max delay value
 *
 * @note if the sub-device is non-audio, AUD_ERR_NOTSUPPORTED will be returned.
 */
aud_error_t
dr_media_device_get_tx_audio_delay_max
(
	const dr_media_device_t * media_device,
	dante_latency_us_t * max_delay
);

/**
 * Sets the value for the additional delay on the TX audio path
 *
 * @param media_device [IN] the media sub-device object
 * @param delay        [IN] delay to be applied
 * @param changes      [IN] pointer that holds TX changed status
 */
aud_error_t
dr_media_device_set_tx_audio_delay
(
	const dr_media_device_t * media_device,
	dante_latency_us_t delay,
	aud_bool_t * changes
);


//----------------------------------------------------------
// TX channel accessors
//----------------------------------------------------------

/**
 * Get the number of tx channels supported by the media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 * @param n            [OUT] number of channels.
 */
uint16_t
dr_media_device_num_txchannels
(
	const dr_media_device_t * media_device
);

/**
 * Get a media sub-device's transmit channel with the high-level identifier.
 * Identifiers start from 1.
 *
 * If the ID doesn't belong to a TX channel of the media sub-device, this call will
 * return AUD_ERR_NOTFOUND.
 *
 * @param media_device [IN] the media sub-device object
 * @param id           [IN] the identifier of the transmit channel to be obtained.
 * @param chan_ptr     [OUT] a pointer to location that holds channel information.
 */
aud_error_t
dr_media_device_txchannel_by_id
(
	const dr_media_device_t * media_device,
	dante_id_t id,
	dr_txchannel_t ** chan_ptr
);

/**
 * Get Dante media type for a given tx channel.
 * Returns dante_media_type_t.
 *
 * @param tx_channel [IN] pointer to tx channel.
 */
dante_media_type_t
dr_txchannel_get_media_type
(
	const dr_txchannel_t * tx_channel
);

/**
 * Get Dante media format for a given transmit channel.
 * Returns dante_media_format_t.
 *
 * @param tx_channel [IN] pointer to transmit channel.
 */
const dante_media_format_t *
dr_txchannel_get_media_format
(
	const dr_txchannel_t * tx_channel
);

/**
 * As dr_device_get_txchannels() for a particular media device
 *
 * @param media_device [IN] the media sub-device object
 * @param num_channels [OUT] a pointer to a location that will hold the number of transmit channels
 * @param channels     [OUT] a pointer to a location that will point to an array of transmit channels
 */
aud_error_t
dr_media_device_get_txchannels
(
	const dr_media_device_t * media_device,
	uint16_t * num_channels,
	dr_txchannel_t *** channels
);

/**
 * As dr_device_get_txchannels(), but can specify which media type is obtained
 *
 * @param device       [IN] the device object
 * @param media_type   [IN] the media type (or DANTE_MEDIA__ALL for all types)
 * @param num_channels [OUT] a pointer to a location that will hold the number of transmit channels
 * @param channels     [OUT] a pointer to a location that will point to an array of transmit channels
 */
aud_error_t
dr_device_get_media_txchannels
(
	const dr_device_t * device,
	dante_media_type_t media_type,
	uint16_t * num_channels,
	dr_txchannel_t *** channels
);


//----------------------------------------------------------
// RX channel accessors
//----------------------------------------------------------

/**
 * Get the number of rx channels supported by the media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 */
uint16_t
dr_media_device_num_rxchannels
(
	const dr_media_device_t * media_device
);

/**
 * Get a media sub-device's receive channel with the high-level identifier.
 * Identifiers start from 1.
 *
 * If the ID doesn't belong to an RX channel of the media sub-device, this call will
 * return AUD_ERR_NOTFOUND.
 *
 * @param media_device [IN] the media sub-device object
 * @param id           [IN] the identifier of the receive channel to be obtained.
 * @param chan_ptr     [OUT] a pointer to location that holds channel information.
 */
aud_error_t
dr_media_device_rxchannel_by_id
(
	const dr_media_device_t * media_device,
	dante_id_t id,
	dr_rxchannel_t ** chan_ptr
);

/**
 * Get Dante media type for a given receive channel.
 * Returns dante_media_type_t.
 *
 * @param rx_channel [IN] pointer to receive channel.
 */
dante_media_type_t
dr_rxchannel_get_media_type
(
	const dr_rxchannel_t * rx_channel
);

/**
 * Get Dante media format for a given receive channel.
 * Returns dante_media_format_t.
 *
 * @param rx_channel [IN] pointer to receive channel.
 */
const dante_media_format_t *
dr_rxchannel_get_media_format
(
	const dr_rxchannel_t * rx_channel
);

/**
 * Get Dante media format options for a given receive channel.
 * Returns dante_media_format_list_t.
 *
 * @param rx_channel [IN] pointer to receive channel.
 */
const dante_media_format_list_t *
dr_rxchannel_get_media_format_options
(
	const dr_rxchannel_t * rx_channel
);

/**
 * Get Dante video format subtype options for a given receive channel.
 *
 * @param rx_channel    [IN] pointer to receive channel.
 * @param len           [OUT] a pointer to a location that will hold the number of video format subtype options
 * @param video_subtypes [OUT] a pointer to a location that will point to an array of video format subtype options
 */
void
dr_rxchannel_get_video_format_subtype_options
(
	const dr_rxchannel_t * rx_channel,
	uint16_t * len,
	dante_video_subtype_t * video_subtypes
);

/**
 * As dr_device_get_rxchannels() for a particular media device
 *
 * @param media_device [IN] the media sub-device object
 * @param num_channels [OUT] a pointer to a location that will hold the number of receive channels
 * @param channels     [OUT] a pointer to a location that will point to an array of receive channels
 */
aud_error_t
dr_media_device_get_rxchannels
(
	const dr_media_device_t * media_device,
	uint16_t * num_channels,
	dr_rxchannel_t *** channels
);

/**
 * As dr_device_get_rxchannels(), but can specify which media type is obtained
 *
 * @param device       [IN] the device object
 * @param media_type   [IN] the media type (or DANTE_MEDIA__ALL for all types)
 * @param num_channels [OUT] a pointer to a location that will hold the number of receive channels
 * @param channels     [OUT] a pointer to a location that will point to an array of receive channels
 */
aud_error_t
dr_device_get_media_rxchannels
(
	const dr_device_t * device,
	dante_media_type_t media_type,
	uint16_t * num_channels,
	dr_rxchannel_t *** channels
);


//----------------------------------------------------------
// TX flow configuration
//----------------------------------------------------------

/**
 * Create a new transmit flow configuration object
 *
 * @param media_device [IN] the media sub-device object
 * @param id           [IN] the media id of the flow to be configured
 * @param num_slots    [IN] the number of slots the flow will have
 * @param config_ptr   [IN] a pointer to the flow configation handle
 *
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_media_txflow_config_new
(
	dr_media_device_t * media_device,
	uint16_t id,
	uint16_t num_slots,
	dr_txflow_config_t ** config_ptr
);

/**
 * Create a new encrypted transmit flow configuration object
 *
 * @param media_device [IN] the media sub-device object
 * @param id           [IN] the media id of the flow to be configured
 * @param num_slots    [IN] the number of slots the flow will have
 * @param config_ptr   [IN] a pointer to the flow configation handle
 *
 * @return AUD_SUCCESS if the encrypted flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_media_txflow_config_new_encrypted
(
	dr_media_device_t * media_device,
	uint16_t id,
	uint16_t num_slots,
	dr_txflow_config_t ** config_ptr
);


/**
 * Obtain the media type
 *
 * @param config the config object
 *
 * @return the media type of the flow being configured
 */
dante_media_type_t
dr_txflow_config_get_media_type
(
	const dr_txflow_config_t * config
);


/**
 * Obtain the media device for this configuration object
 *
 * @param config the config object
 *
 * @return the media device used when the config object was initialised
 */
dr_media_device_t *
dr_txflow_config_get_media_device
(
	dr_txflow_config_t * config
);


//----------------------------------------------------------
// TX flow accessors
//----------------------------------------------------------

/**
 * Get the max number of configurable tx flows for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 */
uint16_t
dr_media_device_max_txflows
(
	const dr_media_device_t * media_device
);

/**
 * Get the number of currently active tx flows for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 */
uint16_t
dr_media_device_num_txflows
(
	const dr_media_device_t * media_device
);

/**
 * Get a media sub-device's transmit flow with the high-level identifier.
 * Identifiers start from 1.
 *
 * If the ID doesn't belong to an active TX flow of the media sub-device, this call will
 * return AUD_ERR_NOTFOUND.
 *
 * @param media_device [IN] the media sub-device object.
 * @param id           [IN] the identifier of the transmit flow to be obtained.
 * @param chan_ptr     [OUT] a pointer to location that holds flow information.
 */
aud_error_t
dr_media_device_txflow_by_id
(
	const dr_media_device_t * media_device,
	uint16_t id,
	dr_txflow_t ** flow_ptr
);

/**
 * Wraps dr_txflows_media_flow_at_index() for media sub-devices.
 *
 * @param media_device [IN] the media sub-device object.
 * @param index        [IN] the index of the transmit flow to be obtained.
 * @param chan_ptr     [OUT] a pointer to location that holds flow information.
 */
aud_error_t
dr_media_device_txflow_at_index
(
	const dr_media_device_t * media_device,
	uint16_t index,
	dr_txflow_t ** flow_ptr
);

/**
 * Get the max transmit flow slots for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object.
 */
uint16_t
dr_media_device_max_txflow_slots
(
	const dr_media_device_t * media_device
);

/**
 * Get Dante media type for a given transmit flow.
 *
 * @param tx_flow        [IN] pointer to transmit flow.
 * @param media_type_ptr [OUT] pointer to hold flow's media type.
 */
aud_error_t
dr_txflow_get_media_type
(
	const dr_txflow_t * tx_flow,
	dante_media_type_t * media_type_ptr
);

/**
 * Get Dante media format for a given transmit flow.
 * Returns dante_media_format_t.
 *
 * @param tx_flow [IN] pointer to transmit flow.
 */
const dante_media_format_t *
dr_txflow_get_media_format
(
	const dr_txflow_t * tx_flow
);

/**
 * Get a handle to a particular media transmit flow by index
 *
 * @param device     [IN] the device object
 * @param media_type [IN] the flow's media type
 * @param index      [IN] the 0-based flow index in the device's data
 * @param flow_ptr   [IN] a pointer to a txflow handle
 */
aud_error_t
dr_device_media_txflow_at_index
(
	dr_device_t * device,
	dante_media_type_t media_type,
	uint16_t index,
	dr_txflow_t ** flow_ptr
);

/**
 * Get a handle to a media transmit flow with the given high-level
 * flow identifier.
 *
 * @param device     [IN] the device object
 * @param media_type [IN] the flow's media type
 * @param id         [IN] the flow's ID
 * @param flow_ptr   [IN] a pointer to a txflow handle
 */
aud_error_t
dr_device_media_txflow_with_id
(
	dr_device_t * device,
	dante_media_type_t media_type,
	dante_id_t id,
	dr_txflow_t ** flow_ptr
);


//----------------------------------------------------------
// RX flow accessors
//----------------------------------------------------------

/**
 * Get the max number of configurable rx flows for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 */
uint16_t
dr_media_device_max_rxflows
(
	const dr_media_device_t * media_device
);

/**
 * Get the number of currently active rx flows for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object
 */
uint16_t
dr_media_device_num_rxflows
(
	const dr_media_device_t * media_device
);

/**
 * Get a media sub-device's receive flow with the high-level identifier.
 * Identifiers start from 1.
 *
 * If the ID doesn't belong to an active RX flow of the media sub-device, this call will
 * return AUD_ERR_NOTFOUND.
 *
 * @param media_device [IN] the media sub-device object
 * @param id           [IN] the identifier of the receive flow to be obtained.
 * @param flow_ptr     [OUT] a pointer to location that holds flow information.
 */
aud_error_t
dr_media_device_rxflow_by_id
(
	const dr_media_device_t * media_device,
	uint16_t id,
	dr_rxflow_t ** flow_ptr
);

/**
 * Wraps dr_rxflows_media_flow_at_index() for media sub-devices.
 *
 * @param media_device [IN] the media sub-device object.
 * @param index        [IN] the index of the receive flow to be obtained.
 * @param chan_ptr     [OUT] a pointer to location that holds flow information.
 */
aud_error_t
dr_media_device_rxflow_at_index
(
	const dr_media_device_t * media_device,
	uint16_t index,
	dr_rxflow_t ** flow_ptr
);

/**
 * Get the max receive flow slots for a media sub-device.
 *
 * @param media_device [IN] the media sub-device object.
 */
uint16_t
dr_media_device_max_rxflow_slots
(
	const dr_media_device_t * media_device
);

/**
 * Get Dante media type for a given receive flow.
 *
 * @param rx_flow [IN] pointer to receive flow.
 * @param media_type_ptr [OUT] pointer to hold flow media type.
 */
aud_error_t
dr_rxflow_get_media_type
(
	const dr_rxflow_t * rx_flow,
	dante_media_type_t * media_type_ptr
);

/**
 * Get Dante media format for a given receive flow.
 * Returns dante_media_format_t.
 *
 * @param rx_flow [IN] pointer to receive flow.
 */
const dante_media_format_t *
dr_rxflow_get_media_format
(
	const dr_rxflow_t * rx_channel
);

/**
 * Get a handle to a particular media receive flow by index
 *
 * @param device     [IN] the device object
 * @param media_type [IN] the flow's media type
 * @param index      [IN] the 0-based flow index in the device's data
 * @param flow_ptr   [IN] a pointer to a rxflow handle
 */
aud_error_t
dr_device_media_rxflow_at_index
(
	dr_device_t * device,
	dante_media_type_t media_type,
	uint16_t index,
	dr_rxflow_t ** flow_ptr
);

/**
 * Get a handle to a media receive flow with the given high-level
 * flow identifier.
 *
 * @param device     [IN] the device object
 * @param media_type [IN] the flow's media type
 * @param id         [IN] the flow's ID
 * @param flow_ptr   [IN] a pointer to a rxflow handle
 */
aud_error_t
dr_device_media_rxflow_with_id
(
	dr_device_t * device,
	dante_media_type_t media_type,
	dante_id_t id,
	dr_rxflow_t ** flow_ptr
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif //_DANTE_ROUTING_MEDIA_H
