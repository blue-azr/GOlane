/*
 * File     : routing_aes67_manual.h
 * Created  : October 2019
 * Author   : Veronica Dantes
 * Synopsis : AES67 manual routing support
 *
 * This software is Copyright (c) 2004-2019, Audinate Pty Ltd and/or its licensors
 *
 **/

#ifndef _ROUTING_AES67_MANUAL_H
#define _ROUTING_AES67_MANUAL_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#include "dante/routing_flows.h"
#include "dante/dante_aes67.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// AES67/SMPTE support
//----------------------------------------------------------

/**
 * Create a new aes67 multicast receive flow configuration object
 *
 * @param device the device on which the flow is being configured
 * @param id the id for this flow. Must be non-zero.
 * @param num_slots the number of slots the flow will have
 * @param config_ptr a pointer to the flow configuration handle
 *
 * @return AUD_SUCCESS if the flow configuration object was successfully
 *   created or an error otherwise
 */
aud_error_t
dr_rxflow_config_new_aes67_multicast_manual
(
	dr_device_t * device,
	uint16_t id,
	uint16_t num_slots,
	dr_rxflow_config_t ** config_ptr
);

/**
 * Set the AES67  media clock offset associated with a RX flow.
 * This field is mandatory to setup an aes67 rx flow.
 * @param config flow configuration
 * @param AES67 media clock offset
 */
aud_error_t
dr_rxflow_config_set_aes67_media_clock_offset
(
	dr_rxflow_config_t * config,
	dante_sdp_media_clock_offset_t media_clock_offset
);

/**
 * Set the AES67 sdp session id & source ipv4 address associated with a RX flow.
 * @param config flow configuration
 * @param AES67 sdp session id
 * @param AES67 sdp source addr
 */
aud_error_t
dr_rxflow_config_set_aes67_stream_id
(
	dr_rxflow_config_t * config,
	dante_sdp_session_id_t session_id,
	uint32_t origin_addr
);


//---------------------

#ifdef __cplusplus
}
#endif

#endif
