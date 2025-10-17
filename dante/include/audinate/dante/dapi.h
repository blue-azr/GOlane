/*
* File     : dapi.h
* Created  : August 2016
* Author   : James Westendorp
* Synopsis : Audinate Specific Environment setup
*
* This software is copyright (c) 2016-2017 Audinate Pty Ltd and/or its licensors
*
* Audinate Copyright Header Version 1 
*/

/**
 * @file dapi.h
 *  Dante API handler types and functions
 */

#ifndef _DAPI_H
#define _DAPI_H

#ifndef DAPI_FLAT_INCLUDE
#include "aud_platform.h"
#include "dante/dante_common.h"
#include "dante/dante_runtime.h"
#include "dante/dante_domains.h"
#include "dante/dapi_types.h"
#include "dante/domain_handler.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @struct dapi_config_t
 * @brief Configuration structure for initializing a DAPI handler.
 *
 * This structure holds configuration parameters for initializing a
 * DAPI handler object. The internal contents of the structure are opaque
 * and should not be accessed or modified directly. Accessor functions
 * like dapi_config_get_* and dapi_config_set_* are advised for use.
 *
 * @note See DAPI example applications for sample usage.
 */
typedef struct dapi_config dapi_config_t;

dapi_config_t *dapi_config_new(void);
void dapi_config_delete(dapi_config_t *config);

/**
 * Get the Dante Domain Handler configuration in the DAPI config object.
 * @param[in] config Reference to the DAPI configuration object
 * @return A reference to the DAPI config's Domain Handler configuration object
 */
dante_domain_handler_config_t *dapi_config_get_domain_handler_config(dapi_config_t *config);

#ifdef WIN32
#   define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PATH 0
#   if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
#       define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT 1
#   else
#       define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT 0
#   endif
#elif defined __linux__ && !defined(microblaze)
#   define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT 0
#   if DAPI_ENVIRONMENT == DAPI_ENVIRONMENT__EMBEDDED
#       define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PATH 1
#   else
#       define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PATH 0
#   endif
#else
#   define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT 0
#   define DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PATH 0
#endif


/**
 * Set the option for mdns clients to connect the a Dante Discovery instance.
 * These functions are only required for Embedded DAPI applications controlling a co-locatable Dante Instance using a private mdns instance
 * @return AUD_SUCCESS on successful mdns connect option configuration
 * @return AUD_ERR_INVALIDPARAMETER if connect option is invalid
 *
 * @note This value is applied across the entire process, not just the current DAPI instance
 */
#if DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PORT == 1
aud_error_t dapi_config_set_mdns_server_port(dapi_config_t * config, uint16_t mdns_server_port);
#elif DAPI_HAS_CONFIGURABLE_MDNS_SERVER_PATH == 1
aud_error_t dapi_config_set_mdns_server_path(dapi_config_t * config, char * mdns_server_path);
#endif

/**
 * @brief Creates a new DAPI handler with default configurations.
 *
 * This function initializes the provided pdapi object using default
 * configurations. It internally calls dapi_new_config() with an
 * empty config structure.
 *
 * @param[out] pdapi Pointer to hold the initialized DAPI handler
 *
 * @note This function is a wrapper around dapi_new_config().
 */
aud_error_t dapi_new(dapi_t **pdapi);

/**
 * @brief Creates a new DAPI handler with the specified configuration.
 *
 * This function initializes the provided pdapi object using the
 * configuration specified in config.
 *
 * Internally, this function will:
 *   1) create the event loop and logs
 *   2) create and initializes the runtime
 *   3) create and initializes the domain handler and
 *   4) create the dante api handler
 *
 * @param[in] config Configuration parameters for the new DAPI handler
 * @param[out] pdapi Pointer to hold the initialized DAPI handler
 */
aud_error_t dapi_new_config(const dapi_config_t *config, dapi_t **pdapi);

/**
 * Deletes the audinate specific envrionment and dante api handler
 */
void dapi_delete(dapi_t * dapi);

/** 
 * Get dante runtime (process data)
 */
dante_runtime_t * dapi_get_runtime(dapi_t * dapi);

/** 
 * Get dante domain handler client
 */
dante_domain_handler_t * dapi_get_domain_handler(dapi_t * dapi);

/**
 * Get the dapi environment for this dapi object
 */
dapi_environment_t dapi_get_dapi_environment(dapi_t * dapi);

/**
 * Get the DAPI library build platform string
 */
const char* dapi_version_info__platform_str(void);

/**
 * Get the DAPI library build variant string
 */
const char* dapi_version_info__variant_str(void);

/**
 * Get the DAPI library build version string
 */
const char* dapi_version_info__version_str(void);

/**
 * Get the DAPI library build timestamp string
 */
const char* dapi_version_info__timestamp_str(void);


#ifdef __cplusplus
}
#endif

#endif
