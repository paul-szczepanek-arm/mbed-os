/* mbed Microcontroller Library
 * Copyright (c) 2017-2018 ARM Limited
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

#ifndef __GENERIC_SECURITY_MANAGER_H__
#define __GENERIC_SECURITY_MANAGER_H__

#include "SecurityManager.h"
#include "PalSecurityManager.h"
#include "Callback.h"
#include "ble/pal/GapTypes.h"

namespace ble {
namespace generic {

using ble::pal::address_t;
using ble::pal::advertising_peer_address_type_t;
using ble::pal::authentication_t;
using ble::pal::key_distribution_t;
using ble::pal::irk_t;
using ble::pal::csrk_t;
using ble::pal::ltk_t;
using ble::pal::ediv_t;
using ble::pal::rand_t;
using ble::pal::pairing_failure_t;
typedef SecurityManager::SecurityIOCapabilities_t SecurityIOCapabilities_t;

class PasskeyNum {
public:
    PasskeyNum() : number() { }

    PasskeyNum(uint32_t num) : number(num) { }

    operator uint32_t() const {
        return number;
    }
private:
    uint32_t number;
};

class PasskeyAsci {
public:
    static const uint8_t NUMBER_OFFSET = '0';

    PasskeyAsci(const uint8_t* passkey) {
        if (passkey) {
            memcpy(asci, passkey, SecurityManager::PASSKEY_LEN);
        } else {
            memset(asci, NUMBER_OFFSET, SecurityManager::PASSKEY_LEN);
        }
    }
    PasskeyAsci() {
        memset(asci, NUMBER_OFFSET, SecurityManager::PASSKEY_LEN);
    }
    PasskeyAsci(const PasskeyNum& passkey) {
        uint32_t num_key = passkey;
        for (int i = 5, m = 100000; i >= 0; --i, m /= 10) {
            uint32_t result = num_key / m;
            asci[i] = NUMBER_OFFSET + result;
            num_key -= result;
        }
    }
    operator PasskeyNum() const {
        return PasskeyNum(getNumber());
    }

    static uint32_t to_num(const uint8_t* asci) {
        uint32_t passkey = 0;
        for (size_t i = 0, m = 1; i < SecurityManager::PASSKEY_LEN; ++i, m *= 10) {
            passkey += (asci[i] - NUMBER_OFFSET) * m;
        }
        return passkey;
    }

private:
    uint32_t getNumber() const {
        return to_num(asci);
    }
    uint8_t asci[SecurityManager::PASSKEY_LEN];
};

/* separate structs to allow db implementation to minimise memory usage */

struct SecurityEntry_t {
    connection_handle_t handle;
    address_t peer_identity_address;
    uint8_t encryption_key_size;
    uint8_t peer_address_public:1;
    uint8_t mitm_protection:1; /**< does the key provide mitm */
    uint8_t keypress_notification:1;
    uint8_t connected:1;
    uint8_t authenticated:1; /**< have we authenticated during this connection */
    uint8_t sign_data:1;
    uint8_t encrypt_data:1;
    uint8_t oob_mitm_protection:1;
    uint8_t oob:1;
    uint8_t secure_connections:1;
};

struct SecurityEntryKeys_t {
    ltk_t  ltk;
    ediv_t ediv;
    rand_t rand;
};

struct SecurityEntryIdentity_t {
    irk_t  irk;
    csrk_t csrk;
};

enum DbCbAction_t {
    DB_CB_ACTION_UPDATE,
    DB_CB_ACTION_NO_UPDATE_REQUIRED, /* does not guarantee discarding changes if you made any */
    DB_CB_ACTION_REMOVE
};

typedef mbed::Callback<DbCbAction_t(SecurityEntry_t&)> SecurityEntryDbCb_t;
typedef mbed::Callback<DbCbAction_t(SecurityEntry_t&, SecurityEntryKeys_t&)> SecurityEntryKeysDbCb_t;
typedef mbed::Callback<DbCbAction_t(SecurityEntry_t&, SecurityEntryIdentity_t&)> SecurityEntryIdentityDbCb_t;
typedef mbed::Callback<DbCbAction_t(Gap::Whitelist_t&)> WhitelistDbCb_t;

class GenericSecurityManagerEventHandler;

/**
 * SecurityDB holds the state for active connections and bonded devices.
 * Keys can be stored in NVM and are returned via callbacks.
 * SecurityDB is responsible for serialising any requests and keeping
 * the store in a consistent state.
 * Active connections state must be returned immediately.
 */
class SecurityDb {
public:
    SecurityDb() {};
    ~SecurityDb() {};

    /**
     * Return immediately security entry containing the state
     * information for active connection.
     * @param[in] handle valid connection handle
     * @return pointer to security entry, NULL if handle was invalid
     */
    SecurityEntry_t* get_entry(connection_handle_t connection);

    void get_entry_keys(SecurityEntryKeysDbCb_t cb, ediv_t ediv, rand_t rand);
    void get_entry_identityt(SecurityEntryIdentityDbCb_t cb, address_t identity_address);

    void update_entry(SecurityEntry_t&);
    void update_entry(connection_handle_t connection,
                      bool address_is_public,
                      address_t &peer_address,
                      ediv_t &ediv,
                      rand_t &rand,
                      ltk_t &ltk,
                      irk_t &irk,
                      csrk_t &csrk);

    void remove_entry(SecurityEntry_t&);
    void clear_entries();

    void get_whitelist(WhitelistDbCb_t cb);

    void update_whitelist(Gap::Whitelist_t&);
    void add_whitelist_entry(address_t);

    void remove_whitelist_entry(address_t);
    void clear_whitelist();

    void restore();
    void sync();
    void setRestore(bool reload);
private:

};

class GenericSecurityManager : public SecurityManager,
                               public ble::pal::SecurityManagerEventHandler {
public:
    ////////////////////////////////////////////////////////////////////////////
    // SM lifecycle management
    //
    ble_error_t init(bool                     initBondable = true,
                     bool                     initMITM     = true,
                     SecurityIOCapabilities_t initIocaps   = IO_CAPS_NONE,
                     const Passkey_t          initPasskey  = NULL) {
        db.restore();
        bondable = initBondable;
        mitm = initMITM;
        iocaps = initIocaps;
        displayPasskey = PasskeyAsci(initPasskey);
        legacyPairingAllowed = true;

        return BLE_ERROR_NONE;
    }

    ble_error_t reset(void) {
        db.sync();

        SecurityManager::reset();

        return BLE_ERROR_NONE;
    }

    ble_error_t preserveBondingStateOnReset(bool enabled) {
        db.setRestore(enabled);
        return BLE_ERROR_NONE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // List management
    //

    ble_error_t purgeAllBondingState(void) {
        db.clear_entries();
        return BLE_ERROR_NONE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Feature support
    //

    ble_error_t allowLegacyPairing(bool allow = true) {
        legacyPairingAllowed = allow;
        return BLE_ERROR_NONE;
    }

    ble_error_t getSecureConnectionsSupport(bool *enabled) {
        return pal.get_secure_connections_support(*enabled);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Security settings
    //

    virtual ble_error_t setDisplayPasskey(const Passkey_t passkey) {
        displayPasskey = passkey;
        return BLE_ERROR_NONE;
    }

    ble_error_t setAuthenticationTimeout(connection_handle_t connection,
                                         uint32_t timeout_in_ms) {
        return pal.set_authentication_timeout(connection, timeout_in_ms / 10);
    }

    ble_error_t getAuthenticationTimeout(connection_handle_t connection,
                                         uint32_t *timeout_in_ms) {
        uint16_t timeout_in_10ms;
        ble_error_t status = pal.get_authentication_timeout(connection, timeout_in_10ms);
        *timeout_in_ms = 10 * timeout_in_10ms;
        return status;
    }

    ble_error_t setLinkSecurity(connection_handle_t connection,
                                SecurityMode_t securityMode) {
        return BLE_ERROR_NOT_IMPLEMENTED;
    }

    ble_error_t getLinkSecurity(connection_handle_t connection,
                                SecurityMode_t *securityMode) {

        *securityMode = SECURITY_MODE_ENCRYPTION_OPEN_LINK;
        return BLE_ERROR_NONE;
    }

    ble_error_t setKeypressNotification(bool enabled = true) {
        keypressNotification = enabled;
        return BLE_ERROR_NONE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Encryption
    //

    /**
     * @deprecated
     *
     * Get the security status of a connection.
     *
     * @param[in]  connection   Handle to identify the connection.
     * @param[out] securityStatusP    Security status.
     *
     * @return BLE_ERROR_NONE or appropriate error code indicating the failure reason.
     */
    ble_error_t getLinkSecurity(connection_handle_t connection, LinkSecurityStatus_t *securityStatusP) {
        return pal.get_encryption_status(connection, *securityStatusP);
    }

    ble_error_t getEncryptionKeySize(connection_handle_t connection, uint8_t *size) {
        SecurityEntry_t *entry = db.get_entry(connection);
        if (entry) {
            *size = entry->encryption_key_size;
            return BLE_ERROR_NONE;
        } else {
            return BLE_ERROR_INVALID_PARAM;
        }
    }

    ////////////////////////////////////////////////////////////////////////////
    // Privacy
    //

    virtual ble_error_t setPrivateAddressTimeout(uint16_t timeout_in_seconds) {
       return pal.set_private_address_timeout(timeout_in_seconds);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Keys
    //

    /**
     * Returns the requested LTK to the PAL.
     *
     * @param entry security entry returned by the database.
     * @param entryKeys security entry containing keys.
     *
     * @return no action instruction to the db since this only reads the keys.
     */
    DbCbAction_t setLtkCb(SecurityEntry_t& entry, SecurityEntryKeys_t& entryKeys) {
        pal.set_ltk(entry.handle, entryKeys.ltk);
        return DB_CB_ACTION_NO_UPDATE_REQUIRED;
    }

    ////////////////////////////////////////////////////////////////////////////
    // Authentication
    //


    ble_error_t requestPairing(connection_handle_t connection) {
        (void) connection;
        return BLE_ERROR_NOT_IMPLEMENTED; /* Requesting action from porters: override this API if security is supported. */
    }

    ble_error_t acceptPairingRequest(connection_handle_t connection) {
        (void) connection;
        return BLE_ERROR_NOT_IMPLEMENTED; /* Requesting action from porters: override this API if security is supported. */
    }

    ble_error_t canceltPairingRequest(connection_handle_t connection) {
        return pal.cancel_pairing(connection, pairing_failure_t::UNSPECIFIED_REASON);
    }

    ble_error_t requestAuthentication(connection_handle_t connection) {
        (void) connection;
        return BLE_ERROR_NOT_IMPLEMENTED; /* Requesting action from porters: override this API if security is supported. */
    }

    ble_error_t setPairingRequestAuthorisation(bool required = true) {
        authorisationRequired = required;
        return BLE_ERROR_NONE;
    }

    ////////////////////////////////////////////////////////////////////////////
    // MITM
    //

    ble_error_t setOOBDataUsage(connection_handle_t connection, bool useOOB, bool OOBProvidesMITM = true) {
        SecurityEntry_t *entry = db.get_entry(connection);
        if (entry) {
            entry->oob = useOOB;
            entry->oob_mitm_protection = OOBProvidesMITM;
            return BLE_ERROR_NONE;
        } else {
            return BLE_ERROR_INVALID_PARAM;
        }
    }

    virtual ble_error_t confirmationEntered(connection_handle_t connection, bool confirmation) {
        return pal.confirmation_entered(connection, confirmation);
    }

    virtual ble_error_t passkeyEntered(connection_handle_t connection, Passkey_t passkey) {
        return pal.passkey_request_reply(connection, PasskeyAsci::to_num(passkey));
    }

    virtual ble_error_t sendKeypressNotification(connection_handle_t connection, Keypress_t keypress) {
        return pal.send_keypress_notification(connection, keypress);
    }

    ////////////////////////////////////////////////////////////////////////////
    // Event handler
    //

    void setSecurityManagerEventHandler(::SecurityManager::SecurityManagerEventHandler* handler) {
        SecurityManager::setSecurityManagerEventHandler(handler);
        if (handler) {
            _app_event_handler = handler;
        }
    }

protected:
    GenericSecurityManager(ble::pal::SecurityManager& palImpl) : pal(palImpl), saveStateEnabled(false) {
        _app_event_handler = &defaultEventHandler;
        pal.set_event_handler(this);
    }

private:
    ble::pal::SecurityManager& pal;
    bool saveStateEnabled;

    SecurityDb db;

    SecurityIOCapabilities_t iocaps;
    PasskeyNum displayPasskey;

    bool mitm;
    bool bondable;
    bool authorisationRequired;
    bool keypressNotification;
    bool oobProvidesMitmProtection;
    bool legacyPairingAllowed;

    authentication_t    authentication;
    uint8_t             minKeySize;
    uint8_t             maxKeySize;
    key_distribution_t  initiatorDist;
    key_distribution_t  responderDist;

    /*  implements ble::pal::SecurityManagerEventHandler */
public:
   void on_security_setup_initiated(connection_handle_t connection, bool allow_bonding,
                                          bool require_mitm, SecurityIOCapabilities_t iocaps) {
        if (_app_event_handler) {
            _app_event_handler->securitySetupInitiated(connection, allow_bonding, require_mitm, iocaps);
        }
    }
   void on_security_setup_completed(connection_handle_t connection,
                                          SecurityManager::SecurityCompletionStatus_t status) {
        if (_app_event_handler) {
            _app_event_handler->securitySetupCompleted(connection, status);
        }
    }
   void on_link_secured(connection_handle_t connection, SecurityManager::SecurityMode_t security_mode) {
        if (_app_event_handler) {
            _app_event_handler->linkSecured(connection, security_mode);
        }
    }

   void on_security_context_stored(connection_handle_t connection) {
        if (_app_event_handler) {
            _app_event_handler->securityContextStored(connection);
        }
    }
   void on_passkey_display(connection_handle_t connection, const SecurityManager::Passkey_t passkey) {
        if (_app_event_handler) {
            _app_event_handler->passkeyDisplay(connection, passkey);
        }
    }

   void on_valid_mic_timeout(connection_handle_t connection) {
        if (_app_event_handler) {
            _app_event_handler->validMicTimeout(connection);
        }
    }

   void on_link_key_failure(connection_handle_t connection) {
        if (_app_event_handler) {
            _app_event_handler->linkKeyFailure(connection);
        }
    }

   void on_keypress_notification(connection_handle_t connection, SecurityManager::Keypress_t keypress) {
        if (_app_event_handler) {
            _app_event_handler->keypressNotification(connection, keypress);
        }
    }

   void on_legacy_pariring_oob_request(connection_handle_t connection) {
        if (_app_event_handler) {
            _app_event_handler->legacyPairingOobRequest(connection);
        }
    }

   void on_oob_request(connection_handle_t connection) {
        if (_app_event_handler) {
            _app_event_handler->oobRequest(connection);
        }
    }
   void on_pin_request(connection_handle_t connection) {

        if (_app_event_handler) {
            _app_event_handler->pinRequest(connection);
        }
    }
   void on_passkey_request(connection_handle_t connection) {

        if (_app_event_handler) {
            _app_event_handler->passkeyRequest(connection);
        }
    }
   void on_confirmation_request(connection_handle_t connection) {

        if (_app_event_handler) {
            _app_event_handler->confirmationRequest(connection);
        }
    }
   void on_accept_pairing_request(connection_handle_t connection,
                                        SecurityIOCapabilities_t iocaps,
                                        bool use_oob,
                                        authentication_t authentication,
                                        uint8_t max_key_size,
                                        key_distribution_t initiator_dist,
                                        key_distribution_t responder_dist) {
       if (_app_event_handler && authorisationRequired) {
            _app_event_handler->acceptPairingRequest(connection);
       }
    }

    void on_keys_distributed(connection_handle_t connection,
                             advertising_peer_address_type_t peer_address_type,
                             address_t &peer_address,
                             ediv_t &ediv,
                             rand_t &rand,
                             ltk_t &ltk,
                             irk_t &irk,
                             csrk_t &csrk) {
        db.update_entry(
            connection,
            (peer_address_type == advertising_peer_address_type_t::PUBLIC_ADDRESS),
            peer_address,
            ediv,
            rand,
            ltk,
            irk,
            csrk
        );
    }

    virtual void on_keys_distributed_ltk(
        connection_handle_t connection,
        ltk_t &ltk
    ) = 0;

    virtual void on_keys_distributed_ediv_rand(
        connection_handle_t connection,
        ediv_t &ediv,
        rand_t &rand
    ) = 0;

    virtual void on_keys_distributed_irk(
        connection_handle_t connection,
        irk_t &irk
    ) = 0;

    virtual void on_keys_distributed_bdaddr(
        connection_handle_t connection,
        advertising_peer_address_type_t peer_identity_address_type,
        address_t &peer_identity_address
    ) = 0;

    virtual void on_keys_distributed_csrk(
        connection_handle_t connection,
        csrk_t &csrk
    ) = 0;

    void on_ltk_request(connection_handle_t connection, ediv_t &ediv, rand_t &rand) {
        db.get_entry_keys(
            mbed::callback(this, &GenericSecurityManager::setLtkCb),
            ediv,
            rand
        );
    }

private:
    /* handler is always a valid pointer */
    ::SecurityManager::SecurityManagerEventHandler *_app_event_handler;
};


} /* namespace generic */
} /* namespace ble */

#endif /*__GENERIC_SECURITY_MANAGER_H__*/
