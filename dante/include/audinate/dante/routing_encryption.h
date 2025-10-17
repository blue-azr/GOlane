/*
* File     : dante/routing_encryption.h
* Created  : April 2020
* Author   : Kingsley Cheung
* Synopsis : Encryption aware routing API
*
* This software is copyright (c) 2008-2025 Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1
*/

#ifndef _DANTE_ROUTING_ENCRYPTION_H
#define _DANTE_ROUTING_ENCRYPTION_H


#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Encryption awareness
//----------------------------------------------------------

/**
 * Is Dante media encryption supported for a device
 *
 * @param device [IN] pointer to device.
 */
aud_bool_t
dr_device_is_dante_media_encryption_supported
(
	const dr_device_t * device
);

/**
 * Is encryption supported for a device?
 *
 * Includes both HDCP and Dante Media Confidentiality
 *
 * @param device [IN] pointer to device.
 */
aud_bool_t
dr_device_is_encryption_supported
(
	const dr_device_t * device
);

/**
 * Is the signal on a given transmit channel encrypted?
 *
 * @param tx_channel [IN] pointer to transmit channel.
 *
 * @return active encryption scheme, or NONE (0) if not encrypted
 */
dante_encryption_scheme_t
dr_txchannel_encrypted_signal
(
	const dr_txchannel_t * tx_channel
);

/**
 * The encryption scheme used by encrypted flows that contain this channel
 *
 * Note that a channel with scheme that is not NONE does not necessarily
 * mean that all flows are required to be encrypted. For the channel's current
 * encryption policy call @ref dr_txchannel_encrypted.
 *
 * @param tx_channel [IN] pointer to transmit channel.
 *
 * @return configured encryption scheme or DANTE_ENCRYPTION__NONE if encryption not
 * supported on this channel.
 */
dante_encryption_scheme_t
dr_txchannel_encryption_scheme
(
	const dr_txchannel_t * tx_channel
);

/**
 * The current encryption policy for this channel
 *
 * If a TX channel has encryption policy enabled then all flows containing
 * the channel MUST be encrypted. The encryption scheme used can be obtained by
 * calling @ref dr_txchannel_encryption_scheme. If the encryption policy is disabled
 * then the channel can go into unencrypted flows but may also be in encrypted flows.
 *
 * @param tx_channel [IN] pointer to transmit channel.
 *
 * @return AUD_TRUE if the channel's encryption policy is enabled and AUD_FALSE
 * otherwise
 */
aud_bool_t
dr_txchannel_encrypted
(
	const dr_txchannel_t * tx_channel
);

/**
 * Is encryption supported for a given receive channel?
 *
 * @param rx_channel [IN] pointer to receive channel.
 *
 * @return supported encryption scheme, or NONE (0) if not supported
 */
dante_encryption_scheme_t
dr_rxchannel_encryption_supported
(
	const dr_rxchannel_t * rx_channel
);

/**
 * Get the encryption scheme for a given transmit flow.
 *
 * @param tx_flow [IN] pointer to transmit flow.
 *
 * @return active encryption scheme, or NONE (0) if not encrypted
 */
dante_encryption_scheme_t
dr_txflow_get_encryption_scheme
(
	const dr_txflow_t * tx_flow
);

/**
 * Get the secure flow group name for an encrypted transmit flow.
 *
 * @param[in] tx_flow pointer to transmit flow
 * @param[out] out_flow_group_name flow group name is returned here
*/
void
dr_txflow_get_secure_flow_group_name
(
	const dr_txflow_t * tx_flow,
	dante_secure_flow_group_name_t out_flow_group_name
);

/**
 * Get the secure flow group name for an encrypted receive flow.
 *
 * @param[in] rx_flow pointer to receive flow
 * @param[out] out_flow_group_name flow group name is returned here
*/
void
dr_rxflow_get_secure_flow_group_name
(
	const dr_rxflow_t * rx_flow,
	dante_secure_flow_group_name_t out_flow_group_name
);

/**
* Get the encryption scheme for a given receive flow.
*
* @param rx_flow [IN] pointer to receive flow.
*
* @return supported encryption scheme, or NONE (0) if not supported
*/
dante_encryption_scheme_t
dr_rxflow_get_encryption_scheme
(
	dr_rxflow_t * rx_flow
);

//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif //_DANTE_ROUTING_ENCRYPTION_H
