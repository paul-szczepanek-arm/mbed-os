/* mbed Microcontroller Library
 * Copyright (c) 2006-2020 ARM Limited
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef BLE_CORDIO_SECURITY_MANAGER_H_
#define BLE_CORDIO_SECURITY_MANAGER_H_

#include <stdint.h>
#include "CallChainOfFunctionPointersWithContext.h"
#include "platform/Callback.h"

#include "ble/types/BLETypes.h"
#include "ble/types/blecommon.h"
#include "ble/Gap.h"

#include "ble/internal/pal/GapTypes.h"
#include "ble/types/BLETypes.h"
#include "ble/internal/SecurityDb.h"
#include "ble/internal/pal/PalConnectionMonitor.h"
#include "ble/internal/cordio/CordioPalSigningMonitor.h"
#include "ble/internal/cordio/CordioPalSecurityManager.h"
#include "ble/SecurityManager.h"
#include "ble/internal/cordio/CordioBLEInstanceBase.h"

namespace ble {

/**
 * Overview
 *
 * Security Manager is used to provide link security through encryption, signing and authentication
 * which are made possible by pairing and optionally bonding. Pairing is the process of establishing
 * and/or exchanging keys used for the current connection. Bonding means saving this information so that
 * it can later be used after reconnecting without having to pair again. This saves time and power.
 *
 * @par Paring
 *
 * There are several ways to provide different levels of security during pairing depending on your requirements
 * and the facilities provided by the application. The process starts with initialising the PalSecurityManager
 * with default options for new connections. Some settings can later be changed per link or globally.
 *
 * The important settings in the init() function are the MITM requirement and IO capabilities. Man in the
 * Middle (MITM) protection prevents an attack where one device can impersonate another device by
 * pairing with both devices at the same time. This protection is achieved by sharing some information
 * between the devices through some independent channel. The IO capabilities of both devices dictate
 * what algorithm is used. For details @see BLUETOOTH SPECIFICATION Version 5.0 | Vol 3, Part H - 2.3.5.1.
 * You can change the IO capabilities after initialisation with setIoCapability(). This will take effect
 * for all subsequent pairings.
 *
 * @par Out of Band data used in pairing
 *
 * Sharing this information through IO capabilities means user interaction which limits the degree of
 * protection due to the limit of the amount of data that we can expect the user to transfer. Another
 * solution is using OOB (out of band) communication to transfer this data instead which can send much
 * more data making MITM attack even less likely to succeed. OOB data has to be exchanged by the application
 * and provided to the Security Manager. Use setOOBDataUsage() to indicate you want to use it. The same call also
 * allows you to set whether or not the communication channel you are using to transmit the OOB data is
 * itself secure against MITM protection - this will set the level of the link security achieved using pairing
 * that uses this data.
 *
 * The most secure pairing is provided by Secure Connections which relies on Elliptical Curve Cryptography.
 * Support for Secure Connections is dependent on both the stack and controller on both sides supporting
 * it. If either side doesn't support it Legacy Pairing will be used. This is an older standard of pairing.
 * If higher security is required legacy pairing can be disabled by calling allowLegacyPairing(false);
 *
 * @par Signing
 *
 * Applications may require a level of security providing confidence that data transfers are coming
 * from a trusted source. This can be achieved by encrypting the link which also provides added confidentiality.
 * Encryption is a good choice when a device stays connected but introduces latency due to the need of encrypting the
 * link if the device only connects periodically to transfer data. If confidentiality is not required data GATT
 * server may allow writes to happen over an unencrypted link but authenticated by a signature present in each packet.
 * This signature relies on having sent a signing key to the peer during pairing prior to sending any signed packets.
 *
 * @par Persistence of Security information
 *
 * Security Manager stores all the data required for its operation on active links. Depending on resources
 * available on the device it will also attempt to store data for disconnected devices which have bonded to be
 * reused when reconnected.
 *
 * If the application has initialised a filesystem and the Security Manager has been provided with a
 * filepath during the init() call it may also provide data persistence across resets. This must be enabled by
 * calling preserveBondingStateOnReset(). Persistence is not guaranteed and may fail if abnormally terminated.
 * The Security Manager may also fall back to a non-persistent implementation if the resources are too limited.
 *
 * @par How to use
 *
 * First thing you need to do is to initialise the manager by calling init() with your chosen settings.
 *
 * The PalSecurityManager communicates with your application through events. These will trigger calls in
 * the EventHandler which you must provide by calling the setSecurityManagerEventHandler() function.
 *
 * The most important process is pairing. This may be triggered manually by calling requestPairing() or
 * may be called as a result of the application requiring encryption by calling setLinkEncryption() or
 * as a result of the application requiring MITM protection through requestAuthentication().
 *
 * All these can be implicitly called by using setLinkSecurity() to conveniently set the required
 * security for the link. The PalSecurityManager will trigger all the process required to achieve the set
 * security level. Security level can only be escalated. Asking the Security Manager for a lower
 * security level than the existing one will not fail but will result in a event informing the
 * application through linkEncryptionResult() of the current level (which remains unchanged).
 *
 * Depending on the IO capabilities and OOB usage settings different pairing algorithms will be chosen.
 * They will produce appropriate events which must be handled by your EventHandler. If your event handler
 * doesn't support all the calls you must not set IO capabilities or set OOB usage in such a way that would
 * trigger them or else the pairing will fail (usually by timing out).
 *
 * The simplest example is a pairing of a device with no IO capabilities and no OOB data available.
 * With such limited pairing capabilities the "just works" method will be employed. This does not provide
 * any MITM protection. The pairing (triggered implicitly or called explicitly) will result in an event
 * being generated on the peer calling pairingRequest(). The event handler must make a decision (either in
 * the application itself or based on user interaction) whether to accept the pairing and call
 * accetPairing() or cancelPairing(). The result will be communicated on both peers through an event calling
 * pairingResult() in the EventHandler.
 *
 * @par Sequence diagrams
 *
 * Sequence diagram "Just Works" pairing
 *
 * \verbatim
 *  /-------- Device 1 ---------\  *----- BLE link -----*  /----------- Device 2-----------\
 *
 * App  EventHandler      PalSecurityManager            PalSecurityManager    EventHandler      App
 *  |        |                  |                          |                 |             |
 *  |-------------------> requestPairing()                 |                 |             |
 *  |        |                  |-----[pairing start]----->|                 |             |
 *  |        |                  |                          |---------> pairingRequest() -->|
 *  |        |                  |                   acceptPairing() <--------------------- |
 *  |        |                  |<--[pairing complete]---->|                 |             |
 *  |<- pairingResult() <-------|                          |---------> pairingResult() --->|
 *  |        |                  |                          |                 |             |
 * @endverbatim
 *
 *  @note the requestPairing() call isn't required to trigger pairing. Pairing will also be triggered
 *  if you request encryption and authentication and no bonding information is available. The sequence will
 *  be the same save for the lack of explicit requestPairing() call.
 *
 *
 *  Sequence diagram Encryption request when bonding information is available
 *
 * \verbatim
 *  /--------- Device 1 ---------\  *------ BLE link ------*  /--------- Device 2 ---------\
 *
 * App  EventHandler       PalSecurityManager              PalSecurityManager   EventHandler    App
 *  |       |                    |                            |                |           |
 *  |--------------------> setLinkEncryption()                |                |           |
 *  |       |                    |<-[encryption established]->|                |           |
 *  |<- linkEncryptionResult() <-|                            |-> linkEncryptionResult() ->|
 *  |       |                    |                            |                |           |
 * @endverbatim
 *
 * @note if bonding information is not available, pairing will be triggered
 *
 *
 * Sequence diagram for Secure Connections passkey entry pairing with one device having a display only
 * and other a keyboard
 *
 * \verbatim
 *  /---- Device 1 (keyboard) ---\  *------ BLE link ------*  /----- Device 2 (display) ---\
 *
 * App  EventHandler       PalSecurityManager              PalSecurityManager  EventHandler     App
 *  |       |                    |                            |               |            |
 *  |--------------------> requestPairing()                   |               |            |
 *  |        |                   |------[pairing start]------>|               |            |
 *  |        |                   |                            |-------> pairingRequest() ->|
 *  |        |                   |                        acceptPairing() <--------------- |
 *  |        |                   |<---[secure con. pairing]-->|               |            |
 *  |<- passkeyRequest() <-------|                            |-------> passkeyDisplay() ->|
 *  |        |                   |                            |               |            |
 *
 *                  user reads the passkey on Device 2 and inputs it on Device 1
 *
 *  |        |                   |                            |               |            |
 *  |------------------->passkeyEntered()                     |               |            |
 *  |        |                   |<---[pairing complete]----->|               |            |
 *  |<- pairingResult() <--------|                            |-------> pairingResult() -->|
 *  |        |                   |                            |               |            |
 * @endverbatim
 *
 */
class SecurityManager :
    public ble::interface::SecurityManager,
    public ble::PalSecurityManagerEventHandler,
    public ble::PalConnectionMonitorEventHandler,
    public ble::PalSigningMonitorEventHandler
{
     // friends
     friend class ble::PalConnectionMonitorEventHandler;
     friend CordioBLEInstanceBase;

    /*
     * The following functions are meant to be overridden in the platform-specific sub-class.
     */
public:
    ////////////////////////////////////////////////////////////////////////////
    // SM lifecycle management
    //

    /**
     * Enable the BLE stack's Security Manager. The Security Manager implements
     * the actual cryptographic algorithms and protocol exchanges that allow two
     * devices to securely exchange data and privately detect each other.
     * Calling this API is a prerequisite for encryption and pairing (bonding).
     *
     * @param[in]  enableBonding Allow for bonding.
     * @param[in]  requireMITM   Require protection for man-in-the-middle attacks.
     * @param[in]  iocaps        To specify the I/O capabilities of this peripheral,
     *                           such as availability of a display or keyboard, to
     *                           support out-of-band exchanges of security data.
     * @param[in]  passkey       To specify a static passkey.
     * @param[in]  signing       Generate and distribute signing key during pairing
     * @param[in]  dbFilepath    Path to the file used to store keys in the filesystem,
     *                           if NULL keys will be only stored in memory
     *
     *
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t init(
	    bool                     enableBonding = true,
        bool                     requireMITM   = true,
        SecurityIOCapabilities_t iocaps        = IO_CAPS_NONE,
        const Passkey_t          passkey       = NULL,
        bool                     signing       = true,
        const char              *dbFilepath    = NULL
    );

    /**
     * Change the file used for the security database. If path is invalid or a NULL is passed
     * keys will only be stored in memory.
     *
     * @note This operation is only allowed with no active connections.
     *
     * @param[in]  dbFilepath    Path to the file used to store keys in the filesystem,
     *                           if NULL keys will be only stored in memory
     *
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t setDatabaseFilepath(const char *dbFilepath = NULL);

    /**
     * Notify all registered onShutdown callbacks that the PalSecurityManager is
     * about to be shutdown and clear all PalSecurityManager state of the
     * associated object.
     *
     * This function is meant to be overridden in the platform-specific
     * sub-class. Nevertheless, the sub-class is only expected to reset its
     * state and not the data held in PalSecurityManager members. This shall be
     * achieved by a call to PalSecurityManager::reset() from the sub-class'
     * reset() implementation.
     *
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t reset(void);

    /**
     * Normally all bonding information is lost when device is reset, this requests that the stack
     * attempts to save the information and reload it during initialisation. This is not guaranteed.
     *
     * @param[in] enable if true the stack will attempt to preserve bonding information on reset.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t preserveBondingStateOnReset(bool enable);

    ////////////////////////////////////////////////////////////////////////////
    // List management
    //

    /**
     * Delete all peer device context and all related bonding information from
     * the database within the security manager.
     *
     * @retval BLE_ERROR_NONE             On success, else an error code indicating reason for failure.
     * @retval BLE_ERROR_INVALID_STATE    If the API is called without module initialization or
     *                                    application registration.
     */
    ble_error_t purgeAllBondingState(void);

    /**
     * Create a list of addresses from all peers in the bond table and generate
     * an event which returns it as a whitelist. Pass in the container for the whitelist.
     * This will be returned by the event.
     *
     * @param[in] whitelist Preallocated whitelist which will be filled up to its capacity.
     *                      If whitelist already contains entries this will be appended to.
     *                      Do not access the whitelist until callback has been called,
     *                      returning the filled whitelist.
     *
     * @retval BLE_ERROR_NONE On success, else an error code indicating reason for failure
     */
    ble_error_t generateWhitelistFromBondTable(::ble::whitelist_t *whitelist) const;

    ////////////////////////////////////////////////////////////////////////////
    // Pairing
    //

    /**
     * Request pairing with the peer. Called by the master.
     * @note Slave can call requestAuthentication or setLinkEncryption to achieve security.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t requestPairing(ble::connection_handle_t connectionHandle);

    /**
     * Accept the pairing request. Called as a result of pairingRequest being called
     * on the event handler.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t acceptPairingRequest(ble::connection_handle_t connectionHandle);

    /**
     * Reject pairing request if the local device is the slave or cancel an outstanding
     * pairing request if master.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t cancelPairingRequest(ble::connection_handle_t connectionHandle);

    /**
     * Tell the stack whether the application needs to authorise pairing requests or should
     * they be automatically accepted.
     *
     * @param[in] required If set to true, pairingRequest in the event handler will
     *                     will be called and will require an action from the application
     *                     to continue with pairing by calling acceptPairingRequest
     *                     or cancelPairingRequest if the user wishes to reject it.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setPairingRequestAuthorisation(bool required = true);

    /**
     * Retrieve identity address for the peer on the given connection.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getPeerIdentity(ble::connection_handle_t connectionHandle);

    ////////////////////////////////////////////////////////////////////////////
    // Feature support
    //

    /**
     * Allow of disallow the use of legacy pairing in case the application only wants
     * to force the use of Secure Connections. If legacy pairing is disallowed and either
     * side doesn't support Secure Connections the pairing will fail.
     *
     * @param[out] allow If true legacy pairing will be used if either side doesn't support Secure Connections.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t allowLegacyPairing(bool allow = true);

    /**
     * Check if the Secure Connections feature is supported by the stack and controller.
     *
     * @param[out] enabled true if SC are supported
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getSecureConnectionsSupport(bool *enabled);

    ////////////////////////////////////////////////////////////////////////////
    // Security settings
    //

    /**
     * Set the IO capability of the local device.
     *
     * @param[in] iocaps type of IO capabilities available on the local device
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setIoCapability(SecurityIOCapabilities_t iocaps);

    /**
     * Set the passkey that is displayed on the local device instead of using
     * a randomly generated one
     *
     * @param[in] passkey ASCII string of 6 digits
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setDisplayPasskey(const Passkey_t passkey);

    /**
     * Set the security mode on a connection. Useful for elevating the security mode
     * once certain conditions are met, e.g., a particular service is found.
     *
     * @param[in]  connectionHandle   Handle to identify the connection.
     * @param[in]  securityMode       Requested security mode.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setLinkSecurity(ble::connection_handle_t connectionHandle, SecurityMode_t securityMode);

    /**
     * Set whether or not we want to send and receive keypress notifications
     * during passkey entry.
     *
     * @param[in] enabled if true pairing will try to enable keypress notifications
     * (dependent on other side supporting it)
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setKeypressNotification(bool enabled = true);

#if BLE_FEATURE_SIGNING
    /**
     * Request generation and exchange of signing keys so that packet signing can be utilised
     * on this connection.
     * @note This does not generate a signingKey event. Use getSigningKey for that.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] enabled          If set to true, signing keys will be exchanged
     *                             during subsequent pairing.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t enableSigning(ble::connection_handle_t connectionHandle, bool enabled = true);
#endif // BLE_FEATURE_SIGNING

    /**
     * Give a hint to the stack that the master/slave role might change in the future.
     *
     * @param[in] enable If set to true it hints the roles are likely to swap in the future.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setHintFutureRoleReversal(bool enable = true);

    /**
     * Set the time after which an event will be generated unless we received a packet with
     * a valid MIC.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] timeout_in_ms Timeout to set.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setAuthenticationTimeout(
        connection_handle_t connection,
        uint32_t timeout_in_ms
    );

    /**
     * Get the time after which an event will be generated unless we received a packet with
     * a valid MIC.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] timeout_in_ms Returns the timeout.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getAuthenticationTimeout(
        connection_handle_t connection,
        uint32_t *timeout_in_ms
    );

    ////////////////////////////////////////////////////////////////////////////
    // Encryption
    //

    /**
     * Current state of encryption on the link.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[out] encryption
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getLinkEncryption(ble::connection_handle_t connectionHandle, ble::link_encryption_t *encryption);

    /**
     * Enabled or disable encryption on the link. The result of this request will be indicated
     * by a call to linkEncryptionResult in the event handler when the action is completed.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] encryption encryption state requested
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setLinkEncryption(ble::connection_handle_t connectionHandle, ble::link_encryption_t encryption);

    /**
     * Set the requirements for encryption key size. If the peer cannot comply with the requirements
     * paring will fail.
     *
     * @param[in] minimumByteSize Smallest allowed encryption key size in bytes. (no smaller than 7)
     * @param[in] maximumByteSize Largest allowed encryption key size in bytes. (no larger than 16)
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setEncryptionKeyRequirements(uint8_t minimumByteSize, uint8_t maximumByteSize);

    /**
     * Get encryption key size for given connection.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[out] size Returns the key size in bits.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getEncryptionKeySize(
        connection_handle_t connectionHandle,
        uint8_t *size
    );

    ////////////////////////////////////////////////////////////////////////////
    // Authentication
    //

    /**
     * Request that the link be authenticated (keys with MITM protection). This might trigger encryption
     * or pairing/re-pairing. The success will be indicated through an event indicating security level change.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t requestAuthentication(ble::connection_handle_t connectionHandle);

    ////////////////////////////////////////////////////////////////////////////
    // MITM
    //

    /**
     * Generate OOB data with the given address. If Secure Connections is supported this will
     * also generate Secure Connections OOB data on top of legacy pairing OOB data. This can be used
     * to generate such data before the connection takes place.
     *
     * In this model the OOB exchange takes place before the devices connect. Devices should establish
     * communication over another channel and exchange the OOB data. The address provided will be used
     * by the peer to associate the received data with the address of the device it will then connect
     * to over BLE.
     *
     * @param[in] address The local address you will use in the connection using this OOB data. This
     *                    address will be returned along with the rest of the OOB data when generation
     *                    is complete. Using an invalid address is illegal.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t generateOOB(const ble::address_t *address);

    /**
     * Enable OOB data usage during paring. If Secure Connections is supported enabling useOOB will
     * generate Secure Connections OOB data through oobGenerated() on top of legacy pairing OOB data.
     *
     * You do not have to call this function to return received OOB data. Use legacyPairingOobReceived
     * or oobReceived to hand it in. This will allow the stack to use it if possible. You only need to
     * call this function to attempt legacy OOB data exchange after pairing start and to inform
     * the stack OOB data does not provide MITM protection (by default it is set to provide this).
     *
     * In this model the OOB exchange takes places after the devices have connected but possibly
     * prior to pairing. For secure connections pairing must not be started until after the OOB
     * data has been sent and/or received. The address in the OOB data generated will match
     * the original address used to establish the connection and will be used by the peer to
     * identify which connection the OOB data belongs to.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] useOOB If set to true, authenticate using OOB data.
     * @param[in] OOBProvidesMITM If set to true keys exchanged during pairing using OOB data
     *                            will provide Man-in-the-Middle protection. This indicates that
     *                            the form of exchange used by the OOB data itself provides MITM
     *                            protection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setOOBDataUsage(ble::connection_handle_t connectionHandle, bool useOOB, bool OOBProvidesMITM = true);

    /**
     * Report to the stack if the passkey matches or not. Used during pairing to provide MITM protection.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] confirmation True value indicates the passkey displayed matches.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t confirmationEntered(ble::connection_handle_t connectionHandle, bool confirmation);

    /**
     * Supply the stack with the user entered passkey.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] passkey ASCII string of digits entered by the user.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t passkeyEntered(ble::connection_handle_t connectionHandle, Passkey_t passkey);

    /**
     * Send a notification to the peer that the user pressed a key on the local device.
     * @note This will only be delivered if the keypress notifications have been enabled during pairing.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] keypress Type of keypress event.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t sendKeypressNotification(ble::connection_handle_t connectionHandle, Keypress_t keypress);

    /**
     * Supply the stack with the OOB data for legacy connections.
     *
     * @param[in] address address of the peer device this data comes from
     * @param[in] tk pointer to out of band data received containing the temporary key.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t legacyPairingOobReceived(const ble::address_t *address, const ble::oob_tk_t *tk);

    /**
     * Supply the stack with the OOB data for secure connections.
     *
     * @param[in] address address of the peer device this data comes from
     * @param[in] random random number used to generate the confirmation
     * @param[in] confirm confirmation value to be use for authentication
     *                    in secure connections pairing
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t oobReceived(const ble::address_t *address, const ble::oob_lesc_value_t *random, const ble::oob_confirm_t *confirm);

    ////////////////////////////////////////////////////////////////////////////
    // Keys
    //

    /**
     * Retrieves a signing key through a signingKey event.
     * If a signing key is not present, pairing/authentication will be attempted.
     * @note This will attempt to retrieve the key even if enableSigning hasn't been called prior to pairing.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] authenticated    Whether the signing key needs to be authenticated
     *                             (provide MITM protection).
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getSigningKey(ble::connection_handle_t connectionHandle, bool authenticated);

    ////////////////////////////////////////////////////////////////////////////
    // Privacy
    //

    /**
     * Sets how often the address is rotated when privacy is enabled.
     *
     * @param[in] timeout_in_seconds How many seconds to wait before starting generation of a new address.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t setPrivateAddressTimeout(
        uint16_t timeout_in_seconds
    );

    /* Event callback handlers. */
public:
    /**
     * Setup a callback to be invoked to notify the user application that the
     * PalSecurityManager instance is about to shutdown (possibly as a result of a call
     * to BLE::shutdown()).
     *
     * @note  It is possible to chain together multiple onShutdown callbacks
     * (potentially from different modules of an application) to be notified
     * before the PalSecurityManager is shutdown.
     *
     * @note  It is also possible to set up a callback into a member function of
     * some object.
     *
     * @note It is possible to unregister a callback using onShutdown().detach(callback)
     */
    void onShutdown(const SecurityManagerShutdownCallback_t& callback);

    template <typename T>
    void onShutdown(T *objPtr, void (T::*memberPtr)(const SecurityManager *));

    /**
     * Provide access to the callchain of shutdown event callbacks.
     * It is possible to register callbacks using onShutdown().add(callback).
     * It is possible to unregister callbacks using onShutdown().detach(callback).
     *
     * @return The shutdown event callbacks chain
     */
    SecurityManagerShutdownCallbackChain_t& onShutdown();

    /**
     * Assign the event handler implementation that will be used by the stack to signal events
     * back to the application.
     *
     * @param[in] handler Event Handler interface implementation.
     */
    void setSecurityManagerEventHandler(EventHandler* handler);

#if !defined(DOXYGEN_ONLY)

    /* ===================================================================== */
    /*                    private implementation follows                     */

    /* implements PalSecurityManager::EventHandler */
public:
    ////////////////////////////////////////////////////////////////////////////
    // Pairing
    //

    /** @copydoc PalSecurityManager::on_pairing_request
     */
    void on_pairing_request(
        connection_handle_t connection,
        bool use_oob,
        AuthenticationMask authentication,
        KeyDistribution initiator_dist,
        KeyDistribution responder_dist
    );

    /** @copydoc PalSecurityManager::on_pairing_error
     */
    void on_pairing_error(
        connection_handle_t connection,
        pairing_failure_t error
    );

    /** @copydoc PalSecurityManager::on_pairing_timed_out
     */
    void on_pairing_timed_out(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_pairing_completed
     */
    void on_pairing_completed(
        connection_handle_t connection
    );

    ////////////////////////////////////////////////////////////////////////////
    // Security
    //

    /** @copydoc PalSecurityManager::on_valid_mic_timeout
     */
    void on_valid_mic_timeout(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_signed_write_received
     */
    void on_signed_write_received(
        connection_handle_t connection,
        uint32_t sign_coutner
    );

    /** @copydoc PalSecurityManager::on_signed_write_verification_failure
     */
    void on_signed_write_verification_failure(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_signed_write
     */
    void on_signed_write();

    /** @copydoc PalSecurityManager::on_slave_security_request
     */
    void on_slave_security_request(
        connection_handle_t connection,
        AuthenticationMask authentication
    );

    ////////////////////////////////////////////////////////////////////////////
    // Encryption
    //

    /** @copydoc PalSecurityManager::on_link_encryption_result
     */
    void on_link_encryption_result(
        connection_handle_t connection,
        link_encryption_t result
    );

    /** @copydoc PalSecurityManager::on_link_encryption_request_timed_out
     */
    void on_link_encryption_request_timed_out(
        connection_handle_t connection
    );

    ////////////////////////////////////////////////////////////////////////////
    // MITM
    //

    /** @copydoc PalSecurityManager::on_passkey_display
     */
    void on_passkey_display(
        connection_handle_t connection,
        passkey_num_t passkey
    );

    /** @copydoc PalSecurityManager::on_keypress_notification
     */
    void on_keypress_notification(
        connection_handle_t connection,
        Keypress_t keypress
    );

    /** @copydoc PalSecurityManager::on_passkey_request
     */
    void on_passkey_request(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_confirmation_request
     */
    void on_confirmation_request(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_secure_connections_oob_request
     */
    void on_secure_connections_oob_request(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_legacy_pairing_oob_request
     */
    void on_legacy_pairing_oob_request(
        connection_handle_t connection
    );

    /** @copydoc PalSecurityManager::on_secure_connections_oob_generated
     */
    void on_secure_connections_oob_generated(
        const oob_lesc_value_t &random,
        const oob_confirm_t &confirm
    );

    ////////////////////////////////////////////////////////////////////////////
    // Keys
    //

    /** @copydoc PalSecurityManager::on_secure_connections_ltk_generated
     */
    void on_secure_connections_ltk_generated(
        connection_handle_t connection,
        const ltk_t &ltk
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_ltk
     */
    void on_keys_distributed_ltk(
        connection_handle_t connection,
        const ltk_t &ltk
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_ediv_rand
     */
    void on_keys_distributed_ediv_rand(
        connection_handle_t connection,
        const ediv_t &ediv,
        const rand_t &rand
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_local_ltk
     */
    void on_keys_distributed_local_ltk(
        connection_handle_t connection,
        const ltk_t &ltk
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_local_ediv_rand
     */
    void on_keys_distributed_local_ediv_rand(
        connection_handle_t connection,
        const ediv_t &ediv,
        const rand_t &rand
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_irk
     */
    void on_keys_distributed_irk(
        connection_handle_t connection,
        const irk_t &irk
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_bdaddr
     */
    void on_keys_distributed_bdaddr(
        connection_handle_t connection,
        advertising_peer_address_type_t peer_address_type,
        const address_t &peer_identity_address
    );

    /** @copydoc PalSecurityManager::on_keys_distributed_csrk
     */
    void on_keys_distributed_csrk(
        connection_handle_t connection,
        const csrk_t &csrk
    );

    /** @copydoc PalSecurityManager::on_ltk_requeston_ltk_request
     */
    void on_ltk_request(
        connection_handle_t connection,
        const ediv_t &ediv,
        const rand_t &rand
    );

    /** @copydoc PalSecurityManager::on_ltk_requeston_ltk_request
     */
    void on_ltk_request(
        connection_handle_t connection
    );

    /* end implements PalSecurityManager::EventHandler */

public:
    SecurityManager(
        PalSecurityManager &palImpl,
        PalConnectionMonitor &connMonitorImpl,
        PalSigningMonitor &signingMonitorImpl
    ) : _pal(palImpl),
        _connection_monitor(connMonitorImpl),
        _signing_monitor(signingMonitorImpl),
        _db(NULL),
        _default_authentication(0),
        _default_key_distribution(KeyDistribution::KEY_DISTRIBUTION_ALL),
        _pairing_authorisation_required(false),
        _legacy_pairing_allowed(true),
        _master_sends_keys(false),
        eventHandler(NULL)
    {
        eventHandler = &defaultEventHandler;
        _pal.set_event_handler(this);

        /* We create a fake value for oob to allow creation of the next oob which needs
         * the last process to finish first before restarting (this is to simplify checking).
         * This fake value will not be used as the oob address is currently invalid */
        _oob_local_random[0] = 1;
    }

    ~SecurityManager()
    {
        delete _db;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Helper functions
    //

private:

    /**
     * Initialise the database, if database already exists it will close it and open the new one.
     *
     * @param db_path path to file to store secure db
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t init_database(const char *db_path = NULL);

    /**
     * Generate identity list based on the database of IRK and apply it to the resolving list.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t init_resolving_list();

    /**
     * Generate the CSRK if needed.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t init_signing();

    /**
     * Generate the IRK if needed.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t init_identity();

    /**
     * Fills the buffer with the specified number of bytes of random data
     * produced by the link controller
     *
     * @param[out] buffer buffer to be filled with random data
     * @param[in] size number of bytes to fill with random data
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t get_random_data(
        uint8_t *buffer,
        size_t size
    );

    /**
     * Send slave security request based on current link settings.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t slave_security_request(
        connection_handle_t connection
    );

    /**
     * Enable encryption on the link, depending on whether device is master or slave.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t enable_encryption(
        connection_handle_t connection
    );

    /**
     * Returns the requested LTK to the PAL. Called by the security db.
     *
     * @param[in] entry security entry returned by the database.
     * @param[in] entryKeys security entry containing keys.
     */
    void enable_encryption_cb(
        SecurityDb::entry_handle_t entry,
        const SecurityEntryKeys_t* entryKeys
    );

    /**
     * Returns the requested LTK to the PAL. Called by the security db.
     *
     * @param[in] entry security entry returned by the database.
     * @param[in] entryKeys security entry containing keys.
     */
    void set_ltk_cb(
        SecurityDb::entry_handle_t entry,
        const SecurityEntryKeys_t* entryKeys
    );

    /**
     * Returns the CSRK for the connection. Called by the security db.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] signing connection signature resolving key and counter.
     */
    void return_csrk_cb(
        SecurityDb::entry_handle_t connection,
        const SecurityEntrySigning_t *signing
    );

    /**
     * Set the peer CSRK for the connection. Called by the security db.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] signing connection signature resolving key and counter.
     */
    void set_peer_csrk_cb(
        SecurityDb::entry_handle_t connection,
        const SecurityEntrySigning_t *signing
    );

    /**
     * Updates the entry for the connection with OOB data presence.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     */
    void update_oob_presence(
        connection_handle_t connection
    );

    /**
     * Set the MITM protection setting on the database entry
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] enable if true set the MITM protection to on.
     */
    void set_mitm_performed(
        connection_handle_t connection,
        bool enable = true
    );

    /**
     * Inform the Security manager of a new connection. This will create
     * or retrieve an existing security manager entry for the connected device.
     * Called by GAP.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @param[in] is_master True if device is the master.
     * @param[in] peer_address_type type of address.
     * @param[in] peer_address Address of the connected device.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    void on_connected(
        connection_handle_t connection,
        connection_role_t role,
        peer_address_type_t peer_address_type,
        address_t peer_address,
        own_address_type_t local_address_type,
        address_t local_address
    );

    /**
     * Inform the security manager that a device has been disconnected and its
     * entry can be put in NVM storage. Called by GAP.
     *
     * @param[in] connectionHandle Handle to identify the connection.
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    void on_disconnected(
        connection_handle_t connection,
        disconnection_reason_t reason
    );

    /**
     * Callback invoked by the secure DB when an identity entry has been
     * retrieved.
     * @param entry Handle of the entry.
     * @param identity The identity associated with the entry; may be NULL.
     */
    void on_security_entry_retrieved(
        SecurityDb::entry_handle_t entry,
        const SecurityEntryIdentity_t* identity
    );

    /**
     * Callback invoked by the secure DB when the identity list has been
     * retrieved.
     *
     * @param identity View to the array passed to the secure DB. It contains
     * identity entries retrieved.
     *
     * @param count Number of identities entries retrieved.
     */
    void on_identity_list_retrieved(
        Span<SecurityEntryIdentity_t>& identity_list,
        size_t count
    );

private:
    struct ControlBlock_t {
        ControlBlock_t();

        KeyDistribution get_initiator_key_distribution() {
            return KeyDistribution(initiator_key_distribution);
        };
        KeyDistribution get_responder_key_distribution() {
            return KeyDistribution(responder_key_distribution);
        };
        void set_initiator_key_distribution(KeyDistribution mask) {
            initiator_key_distribution = mask.value();
        };
        void set_responder_key_distribution(KeyDistribution mask) {
            responder_key_distribution = mask.value();
        };

        connection_handle_t connection;
        SecurityDb::entry_handle_t db_entry;

        address_t local_address; /**< address used for connection, possibly different from identity */

    private:
        uint8_t initiator_key_distribution:4;
        uint8_t responder_key_distribution:4;
    public:
        uint8_t connected:1;
        uint8_t authenticated:1; /**< have we turned encryption on during this connection */
        uint8_t is_master:1;

        uint8_t encryption_requested:1;
        uint8_t encryption_failed:1;
        uint8_t encrypted:1;
        uint8_t signing_requested:1;
        uint8_t signing_override_default:1;

        uint8_t mitm_requested:1;
        uint8_t mitm_performed:1; /**< keys exchange will have MITM protection */

        uint8_t attempt_oob:1;
        uint8_t oob_mitm_protection:1;
        uint8_t oob_present:1;
        uint8_t legacy_pairing_oob_request_pending:1;

        uint8_t csrk_failures:2;
    };

    /* list management */

    ControlBlock_t* acquire_control_block(connection_handle_t connection);

    ControlBlock_t* get_control_block(connection_handle_t connection);

    ControlBlock_t* get_control_block(const address_t &peer_address);

    ControlBlock_t* get_control_block(SecurityDb::entry_handle_t db_entry);

    void release_control_block(ControlBlock_t* entry);

private:
    SecurityManagerShutdownCallbackChain_t shutdownCallChain;
    EventHandler* eventHandler;
    EventHandler  defaultEventHandler;

    PalSecurityManager &_pal;
    PalConnectionMonitor &_connection_monitor;
    PalSigningMonitor &_signing_monitor;

    SecurityDb *_db;

    /* OOB data */
    address_t _oob_local_address;
    address_t _oob_peer_address;
    oob_lesc_value_t _oob_peer_random;
    oob_confirm_t _oob_peer_confirm;
    oob_lesc_value_t _oob_local_random;
    address_t _oob_temporary_key_creator_address; /**< device which generated and sent the TK */
    oob_tk_t _oob_temporary_key; /**< used for legacy pairing */

    AuthenticationMask _default_authentication;
    KeyDistribution _default_key_distribution;

    bool _pairing_authorisation_required;
    bool _legacy_pairing_allowed;
    bool _master_sends_keys;

    static const size_t MAX_CONTROL_BLOCKS = 5;
    ControlBlock_t _control_blocks[MAX_CONTROL_BLOCKS];
#endif // !defined(DOXYGEN_ONLY)
};

} // ble

#endif /*BLE_CORDIO_SECURITY_MANAGER_H_*/
