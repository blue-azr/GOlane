/*
 * File     : routing_batch_config.h
 * Created  : September 2019
 * Author   : Andrew White
 * Synopsis : Batch-oriented routing types and functions.
 *
 * Copyright 2023 Audinate Pty Ltd and/or its licensors
 * 
 */

/**
 * @file routing_batch_config.h
 *
 * The batch API allows configuration of multiple resources in a single 'request'.
 * Each batch request applies the same operation to the same type of resource,
 * for example unsubscribing channels or deleting flows.
 *
 * See \ref dr_device_batch_config_t for workflow of a batch configuration request.
 */
#ifndef _ROUTING_BATCH_CONFIG_H
#define _ROUTING_BATCH_CONFIG_H

#ifndef DAPI_FLAT_INCLUDE
#include "dante/routing.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

//----------------------------------------------------------
// Batch configuration types
//----------------------------------------------------------

/**
 * Object for describing a batch configuration request.
 *
 * Batch configuration objects are always created and populated by a functions
 * specific to the operation being performed.  Commit and memory management
 * functions apply to all batch configuration operations.
 *
 * @note batch configuration objects are reference tracked.  Committing a
 * batch configuration object adds a reference on behalf of the client code.
 * Client code must explicitly call 'dr_batch_config_release' once it no
 * longer needs the object.
 *
 * Note that this behaviour is different to multicast flow config objects.
 *
 * @note releasing a batch configuration object does not terminate an in-flight
 * request.
 *
 * Workflow of a batch request:
 * -# initialise a batch configuration object, defining
 *   - the type of resource (e.g. rxchannels)
 *   - the operation to perform (e.g. unsubscribe)
 *    The newly constructed batch configuration object is in the preparing state
 * -# add resources to the batch configuration
 * -# commit the batch configuration as a request
 *   - The batch configuration object is finished preparing and becomes read-only.
 *   - Client code can (and usually should) now release the batch configuration object.
 *     Dante routing API will manage it from now on.
 * -# Dante routing API completes the request and fires the completion callback, if any
 */
typedef struct dr_device_batch_config dr_device_batch_config_t;


//----------------------------------------------------------
// Shared Operations
//----------------------------------------------------------

/**
 * Commit a batch configuration request
 *
 * @param config the batch configuration request object to commit
 *
 * @note this function does NOT release ownership of config.
 * See dr_batch_config_release()
 * @note a successful call to commit will mark the batch configuration object
 * as read-only.  See dr_batch_config_is_preparing().
 */
aud_error_t
dr_batch_config_commit
(
	dr_device_batch_config_t * config,
	dr_device_response_fn * response_fn,
	dante_request_id_t * request_id
);

/**
 * Release batch configuration memory acquired via constructor
 *
 * @param config the batch configuration request object acquired by a constructor
 */
aud_error_t
dr_batch_config_release
(
	dr_device_batch_config_t * config
);


//----------------------------------------------------------
// Constructors
//----------------------------------------------------------

/**
 * Create a new batch configuration request object for RX flow delete
 *
 * @param device the device to configure
 *
 * Supported 'add' functions
 * - dr_batch_config_add_rxflow()
 */
dr_device_batch_config_t *
dr_device_batch_new_rxflow_delete
(
	dr_device_t * device
);

/**
 * Create a new batch configuration request object for TX flow delete
 *
 * @param device the device to configure
 *
 * Supported 'add' functions
 * - dr_batch_config_add_txflow()
 */
dr_device_batch_config_t *
dr_device_batch_new_txflow_delete
(
	dr_device_t * device
);

/**
 * Create a new batch configuration request object for RX rename
 *
 * @param device the device to configure
 *
 * Supported 'add' functions
 * - dr_batch_config_set_rxchannel_name()
 * - dr_batch_config_set_rxchannel_name_by_id()
 */
dr_device_batch_config_t *
dr_device_batch_new_rxchannel_rename
(
	dr_device_t * device
);

/**
 * Create a new batch configuration request object for TX rename
 *
 * @param device the device to configure
 *
 * Supported 'add' functions
 * - dr_batch_config_set_txchannel_name()
 * - dr_batch_config_set_txchannel_name_by_id()
 */
dr_device_batch_config_t *
dr_device_batch_new_txchannel_rename
(
	dr_device_t * device
);

/**
 * Create a new batch configuration request object for RX subscribe
 *
 * @param device the device to configure
 *
 * Supported 'add' functions
 * - dr_batch_config_set_rxchannel_subscription()
 * - dr_batch_config_set_rxchannel_subscription_by_id()
 */
dr_device_batch_config_t *
dr_device_batch_new_rxchannel_subscribe
(
	dr_device_t * device
);


//----------------------------------------------------------
// Add functions
//----------------------------------------------------------

/**
 * Add an RX flow to a batch configuration that supports RX flows
 *
 * @param config the batch configuration request object
 * @param rxflow the rxflow to add
 *
 * Supported by:
 * - dr_device_batch_new_rxflow_delete()
 */
aud_error_t
dr_batch_config_add_rxflow
(
	dr_device_batch_config_t * config,
	const dr_rxflow_t * rxflow
);

/**
 * Add an RX flow to a batch configuration that supports RX flows, by ID
 *
 * @param config the batch configuration request object
 * @param media_type the media type of the rxflow being added
 * @param flow_id the ID of the rxflow being added
 *
 * @note this function does not verify that the given flow exists
 */
aud_error_t
dr_batch_config_add_rxflow_by_id
(
	dr_device_batch_config_t * config,
	dante_media_type_t media_type,
	dante_id_t flow_id
);


/**
 * Add a TX flow to a batch configuration that supports TX flows
 *
 * @param config the batch configuration request object
 * @param txflow the txflow to add
 *
 * Supported by:
 * - dr_device_batch_new_txflow_delete()
 */
aud_error_t
dr_batch_config_add_txflow
(
	dr_device_batch_config_t * config,
	const dr_txflow_t * txflow
);

/**
 * Add a TX flow to a batch configuration that supports TX flows, by ID
 *
 * @param config the batch configuration request object
 * @param media_type the media type of the txflow being added
 * @param flow_id the ID of the txflow being added
 *
 * @note this function does not verify that the given flow exists
 */
aud_error_t
dr_batch_config_add_txflow_by_id
(
	dr_device_batch_config_t * config,
	dante_media_type_t media_type,
	dante_id_t flow_id
);


//----------------------------------------------------------
// Set functions
//----------------------------------------------------------

// These are the same as 'add' functions, but with a payload not just an ID

/**
 * In the batch configuration, set the name of a RX channel
 *
 * @param config the batch configuration request object
 * @param channel the rxchannel to configure
 * @param name new channel name, or NULL to reset to default
 *
 * Supported by:
 * - dr_device_batch_new_rxchannel_rename()
 */
aud_error_t
dr_batch_config_set_rxchannel_name
(
	dr_device_batch_config_t * config,
	const dr_rxchannel_t * channel,
	const char * name
);

/**
 * In the batch configuration, set the name of a RX channel, by ID
 *
 * @param config the batch configuration request object
 * @param media_type the media type of the rxchannel being configured
 * @param channel_id the ID of the rxchannel being configured
 * @param name new channel name, or NULL to reset to default
 *
 * @note this function does not verify that the given channel exists
 *
 * Supported by:
 * - dr_device_batch_new_rxchannel_rename()
 */
aud_error_t
dr_batch_config_set_rxchannel_name_by_id
(
	dr_device_batch_config_t * config,
	dante_media_type_t media_type,
	dante_id_t channel_id,
	const char * name
);

/**
 * In the batch configuration, set the name of a TX channel.
 * If name matches any TX channel's default (canonical) name,
 * the channel's name to will be set to default.
 *
 * @param config the batch configuration request object
 * @param channel the txchannel to configure
 * @param name new channel name, or NULL to reset to default
 *
 * Supported by:
 * - dr_device_batch_new_txchannel_rename()
 */
aud_error_t
dr_batch_config_set_txchannel_name
(
	dr_device_batch_config_t * config,
	const dr_txchannel_t * channel,
	const char * name
);

/**
 * In the batch configuration, set the name of a TX channel, by ID.
 * If name matches any TX channel's default (canonical) name,
 * the channel's name to will be set to default.
 *
 * @param config the batch configuration request object
 * @param media_type the media type of the txchannel being configured
 * @param channel_id the ID of the txchannel being configured
 * @param name new channel name, or NULL to reset to default
 *
 * @note this function does not verify that the given channel exists
 *
 * Supported by:
 * - dr_device_batch_new_txchannel_rename()
 */
aud_error_t
dr_batch_config_set_txchannel_name_by_id
(
	dr_device_batch_config_t * config,
	dante_media_type_t media_type,
	dante_id_t channel_id,
	const char * name
);

/**
 * In the batch configuration, set the Dante subscription of an RX channel
 *
 * @param config the batch configuration request object
 * @param channel the rxchannel to configured
 * @param tx_device the tx device to subscribe to, or NULL to clear the subscription
 * @param tx_channel the tx channel to subscribe to
 *
 * @note this function does not verify that the given channel exists
 *
 * Supported by:
 * - dr_device_batch_new_rxchannel_subscribe()
 */
aud_error_t
dr_batch_config_set_rxchannel_subscription
(
	dr_device_batch_config_t * config,
	const dr_rxchannel_t * channel,
	const char * tx_device,
	const char * tx_channel
);

/**
 * In the batch configuration, set the Dante subscription of an RX channel, by ID
 *
 * @param config the batch configuration request object
 * @param media_type the media type of the rxchannel being configured
 * @param channel_id the ID of the rxchannel being configured
 * @param tx_device the tx device to subscribe to, or NULL to clear the subscription
 * @param tx_channel the tx channel to subscribe to
 *
 * @note this function does not verify that the given channel exists
 *
 * Supported by:
 * - dr_device_batch_new_rxchannel_subscribe()
 */
aud_error_t
dr_batch_config_set_rxchannel_subscription_by_id
(
	dr_device_batch_config_t * config,
	dante_media_type_t media_type,
	dante_id_t channel_id,
	const char * tx_device,
	const char * tx_channel
);

//----------------------------------------------------------
// Utility functions
//----------------------------------------------------------

/**
 * Return the device associated with the current batch configuration
 */
dr_device_t *
dr_batch_config_get_device
(
	const dr_device_batch_config_t * config
);

/**
 * Is the batch configuration object in the preparing state?
 *
 * A batch configuration object is in the preparing state after it has been
 * constructed but before commit is successfully called.  A batch configuration
 * can only be modified while preparing.
 *
 * @param config the batch configuration request object
 * @return TRUE if the batch configuration is preparing
 *
 * @note all add operations will fail once the batch configuration leaves the
 * preparing state.
 */
aud_bool_t
dr_batch_config_is_preparing
(
	const dr_device_batch_config_t * config
);

/**
 * Get the number of pending messages (packets) to be sent for the batch config
 *
 * @param config  the batch configuration request object
 *
 * @note for uncomitted batch configs, this will return the total number of packets
 * created to send all entries.
 */
uint16_t
dr_batch_config_num_pending_msgs
(
	const dr_device_batch_config_t * config
);

/**
 * Obtain a reference to the batch configuration used for the request.
 *
 * @param device the device object
 * @param request_id the request ID object
 *
 * @return reference to the batch configuration passed in dr_batch_config_commit()
 * IF this request was a commit of a batch configuration AND the configuration
 * is still held by the request, or NULL if not available or applicable.
 */
dr_device_batch_config_t *
dr_device_request_get_batch_config
(
	dr_device_t * device,
	dante_request_id_t request_id
);

/**
 * Clone (copy) a batch configuration request.
 *
 * The copy contains the same data as the original, but is in the preparing state.
 * The copy is a new batch configuration and must be released using dr_batch_config_release()
 * when done.
 *
 * @param config the batch configuration request object to copy
 * @return a new batch configuration object matching the input batch configuration
 */
dr_device_batch_config_t *
dr_batch_config_new_clone
(
	const dr_device_batch_config_t * config
);


//----------------------------------------------------------

#ifdef __cplusplus
}
#endif

#endif
