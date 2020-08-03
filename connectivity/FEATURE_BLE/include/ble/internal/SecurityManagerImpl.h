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

#ifndef IMPL_SECURITY_MANAGER_H_
#define IMPL_SECURITY_MANAGER_H_

#include <stdint.h>
#include "CallChainOfFunctionPointersWithContext.h"
#include "platform/Callback.h"

#include "ble/types/BLETypes.h"
#include "ble/types/blecommon.h"
#include "ble/Gap.h"

#include "ble/internal/GapTypes.h"
#include "ble/types/BLETypes.h"
#include "ble/internal/SecurityDb.h"
#include "ble/internal/PalConnectionMonitor.h"
#include "ble/internal/PalSigningMonitor.h"
#include "ble/internal/PalSecurityManager.h"
#include "ble/SecurityManager.h"
#include "ble/internal/BLEInstanceBase.h"

namespace ble {

class SecurityManager :
    public ble::interface::SecurityManager,
    public ble::PalSecurityManagerEventHandler,
    public ble::PalConnectionMonitorEventHandler,
    public ble::PalSigningMonitorEventHandler
{
    friend class ble::PalConnectionMonitorEventHandler;
    friend BLEInstanceBase;
    friend PalGenericAccessService;
    friend PalSecurityManager;

public:
    ////////////////////////////////////////////////////////////////////////////
    // SM lifecycle management
    //

    ble_error_t init(
	    bool                     enableBonding = true,
        bool                     requireMITM   = true,
        SecurityIOCapabilities_t iocaps        = IO_CAPS_NONE,
        const Passkey_t          passkey       = NULL,
        bool                     signing       = true,
        const char              *dbFilepath    = NULL
    );

    ble_error_t setDatabaseFilepath(const char *dbFilepath = NULL);

    ble_error_t reset(void);

    ble_error_t preserveBondingStateOnReset(bool enable);

    ////////////////////////////////////////////////////////////////////////////
    // List management
    //

    ble_error_t purgeAllBondingState(void);

    ble_error_t generateWhitelistFromBondTable(::ble::whitelist_t *whitelist) const;

    ////////////////////////////////////////////////////////////////////////////
    // Pairing
    //

    ble_error_t requestPairing(ble::connection_handle_t connectionHandle);

    ble_error_t acceptPairingRequest(ble::connection_handle_t connectionHandle);

    ble_error_t cancelPairingRequest(ble::connection_handle_t connectionHandle);

    ble_error_t setPairingRequestAuthorisation(bool required = true);

    ble_error_t getPeerIdentity(ble::connection_handle_t connectionHandle);

    ////////////////////////////////////////////////////////////////////////////
    // Feature support
    //

    ble_error_t allowLegacyPairing(bool allow = true);

    ble_error_t getSecureConnectionsSupport(bool *enabled);

    ////////////////////////////////////////////////////////////////////////////
    // Security settings
    //

    ble_error_t setIoCapability(SecurityIOCapabilities_t iocaps);

    ble_error_t setDisplayPasskey(const Passkey_t passkey);

    ble_error_t setLinkSecurity(ble::connection_handle_t connectionHandle, SecurityMode_t securityMode);

    ble_error_t setKeypressNotification(bool enabled = true);

#if BLE_FEATURE_SIGNING

    ble_error_t enableSigning(ble::connection_handle_t connectionHandle, bool enabled = true);
#endif // BLE_FEATURE_SIGNING

    ble_error_t setHintFutureRoleReversal(bool enable = true);

    ble_error_t setAuthenticationTimeout(
        connection_handle_t connection,
        uint32_t timeout_in_ms
    );

    ble_error_t getAuthenticationTimeout(
        connection_handle_t connection,
        uint32_t *timeout_in_ms
    );

    ////////////////////////////////////////////////////////////////////////////
    // Encryption
    //

    ble_error_t getLinkEncryption(ble::connection_handle_t connectionHandle, ble::link_encryption_t *encryption);

    ble_error_t setLinkEncryption(ble::connection_handle_t connectionHandle, ble::link_encryption_t encryption);

    ble_error_t setEncryptionKeyRequirements(uint8_t minimumByteSize, uint8_t maximumByteSize);

    ble_error_t getEncryptionKeySize(
        connection_handle_t connectionHandle,
        uint8_t *size
    );

    ////////////////////////////////////////////////////////////////////////////
    // Authentication
    //

    ble_error_t requestAuthentication(ble::connection_handle_t connectionHandle);

    ////////////////////////////////////////////////////////////////////////////
    // MITM
    //

    ble_error_t generateOOB(const ble::address_t *address);

    ble_error_t setOOBDataUsage(ble::connection_handle_t connectionHandle, bool useOOB, bool OOBProvidesMITM = true);

    ble_error_t confirmationEntered(ble::connection_handle_t connectionHandle, bool confirmation);

    ble_error_t passkeyEntered(ble::connection_handle_t connectionHandle, Passkey_t passkey);

    ble_error_t sendKeypressNotification(ble::connection_handle_t connectionHandle, ble::Keypress_t keypress);

    ble_error_t legacyPairingOobReceived(const ble::address_t *address, const ble::oob_tk_t *tk);

    ble_error_t oobReceived(const ble::address_t *address, const ble::oob_lesc_value_t *random, const ble::oob_confirm_t *confirm);

    ////////////////////////////////////////////////////////////////////////////
    // Keys
    //

    ble_error_t getSigningKey(ble::connection_handle_t connectionHandle, bool authenticated);

    ////////////////////////////////////////////////////////////////////////////
    // Privacy
    //

    ble_error_t setPrivateAddressTimeout(
        uint16_t timeout_in_seconds
    );

    /* Event callback handlers. */
public:

    void onShutdown(const SecurityManagerShutdownCallback_t& callback);

    template <typename T>
    void onShutdown(T *objPtr, void (T::*memberPtr)(const SecurityManager *));

    SecurityManagerShutdownCallbackChain_t& onShutdown();

    void setSecurityManagerEventHandler(EventHandler* handler);

    /* ===================================================================== */
    /*                    private implementation follows                     */

    /* implements PalSecurityManager::EventHandler */
private:
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
        ble::Keypress_t keypress
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

private:
    /* Disallow copy and assignment. */
    SecurityManager(const SecurityManager &);
    SecurityManager& operator=(const SecurityManager &);

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

    ble_error_t init_database(const char *db_path = NULL);

    ble_error_t init_resolving_list();

    ble_error_t init_signing();

    ble_error_t init_identity();

    ble_error_t get_random_data(
        uint8_t *buffer,
        size_t size
    );

    ble_error_t slave_security_request(
        connection_handle_t connection
    );

    ble_error_t enable_encryption(
        connection_handle_t connection
    );

    void enable_encryption_cb(
        ble::SecurityDb::entry_handle_t entry,
        const SecurityEntryKeys_t* entryKeys
    );

    void set_ltk_cb(
        SecurityDb::entry_handle_t entry,
        const SecurityEntryKeys_t* entryKeys
    );

    void return_csrk_cb(
        SecurityDb::entry_handle_t connection,
        const SecurityEntrySigning_t *signing
    );

    void set_peer_csrk_cb(
        SecurityDb::entry_handle_t connection,
        const SecurityEntrySigning_t *signing
    );

    void update_oob_presence(
        connection_handle_t connection
    );

    void set_mitm_performed(
        connection_handle_t connection,
        bool enable = true
    );

    void on_connected(
        connection_handle_t connection,
        connection_role_t role,
        peer_address_type_t peer_address_type,
        address_t peer_address,
        own_address_type_t local_address_type,
        address_t local_address
    );

    void on_disconnected(
        connection_handle_t connection,
        disconnection_reason_t reason
    );

    void on_security_entry_retrieved(
        SecurityDb::entry_handle_t entry,
        const SecurityEntryIdentity_t* identity
    );

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
};

} // ble

#endif /*IMPL_SECURITY_MANAGER_H_*/
