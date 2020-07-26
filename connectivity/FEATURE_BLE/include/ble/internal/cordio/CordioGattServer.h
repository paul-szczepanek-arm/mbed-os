/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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

#ifndef MBED_CORDIO_GATT_SERVER_H__
#define MBED_CORDIO_GATT_SERVER_H__

#include "CallChainOfFunctionPointersWithContext.h"

#include "ble/types/GattService.h"
#include "ble/types/GattAttribute.h"
#include "ble/types/GattServerEvents.h"
#include "ble/types/GattCallbackParamTypes.h"

#include <stddef.h>
#include "ble/types/blecommon.h"
#include "pal/SigningMonitor.h"
#include "ble/Gap.h"
#include "wsf_types.h"
#include "att_api.h"
#include "SecurityManager.h"

#include "ble/BLE.h"
#include "ble/internal/pal/SigningMonitor.h"

/*! Maximum count of characteristics that can be stored for authorisation purposes */
#define MAX_CHARACTERISTIC_AUTHORIZATION_CNT 20

/*! client characteristic configuration descriptors settings */
#define MAX_CCCD_CNT 20

namespace ble {

// fwd declaration of CordioPalAttClient and BLE
namespace pal {
class CordioPalAttClient;
}
class BLE;
class CordioSigningMonitor;

/**
 * @addtogroup ble
 * @{
 * @addtogroup gatt
 * @{
 * @addtogroup server
 * @{
 */

/**
 * Construct and operates a GATT server.
 *
 * A Gatt server is a collection of GattService; these services contain
 * characteristics that a peer connected to the device may read or write.
 * These characteristics may also emit updates to subscribed clients when their
 * values change.
 *
 * @p Server Layout
 *
 * Application code can add a GattService object to the server with the help of
 * the function addService(). That function registers all the GattCharacteristic
 * enclosed in the service, as well as all the characteristics descriptors (see
 * GattAttribute) these characteristics contain. Service registration assigns
 * a unique handle to the various attributes being part of the service; this
 * handle should be used for subsequent read or write of these components.
 *
 * There are no primitives defined to remove a single service; however, a call to
 * the function reset() removes all services previously registered in the
 * GattServer.
 *
 * @p Characteristic and attributes access
 *
 * Values of the characteristic and the characteristic descriptor present in the
 * GattServer must be accessed through the handle assigned to them when the service
 * has been registered; the GattServer class offers several flavors of read()
 * and write() functions that retrieve or mutate an attribute value.
 *
 * Application code can query if a client has subscribed to a given
 * characteristic's value update by invoking the function areUpdatesEnabled().
 *
 * @p Events
 *
 * The GattServer allows application code to register several event handlers that
 * can be used to monitor client and server activities:
 *   - onDataSent(): Register an event handler that is called when a
 *     characteristic value update has been sent to a client.
 *   - onDataWriten(): Register an event handler that is called when a
 *     client has written an attribute of the server.
 *   - onDataRead(): Register an event handler that is called when a
 *     client has read an attribute of the server.
 *   - onUpdatesEnabled: Register an event handler that is called when a
 *     client subscribes to updates of a characteristic.
 *   - onUpdatesDisabled: Register an event handler that is called when a
 *     client unsubscribes from updates of a characteristic.
 *   - onConfimationReceived: Register an event handler that is called
 *     when a client acknowledges a characteristic value notification.
 *
 * @note The term characteristic value update is used to represent
 * Characteristic Value Notification and Characteristic Value Indication when
 * the nature of the server initiated is not relevant.
 */
class GattServer :
        public ble::interface::GattServer,
        public pal::SigningMonitor {
    friend ble::BLE;
    friend ble::pal::CordioPalAttClient;
    friend CordioSigningMonitor;
    friend CordioBLEInstanceBase;

// inherited typedefs have the wrong types so we have to redefine them
public:
    /**
     * Event handler invoked when the GattServer is reset.
     *
     * @see onShutdown() reset()
     */
    typedef FunctionPointerWithContext<const GattServer *>
        GattServerShutdownCallback_t;

    /**
     * Callchain of GattServerShutdownCallback_t.
     *
     * @see onShutdown() reset()
     */
    typedef CallChainOfFunctionPointersWithContext<const GattServer*>
        GattServerShutdownCallbackChain_t;
public:
    /**
     * Assign the event handler implementation that will be used by the
     * module to signal events back to the application.
     *
     * @param handler Application implementation of an EventHandler.
     */
    void setEventHandler(EventHandler *handler);

    /**
     * Shut down the GattServer instance.
     *
     * This function notifies all event handlers listening for shutdown events
     * that the GattServer is about to be shut down; then it clears all
     * GattServer state.
     *
     * @note This function is meant to be overridden in the platform-specific
     * subclass. Overides must call the parent function before any cleanup.
     *
     * @return BLE_ERROR_NONE on success.
     */
    ble_error_t reset(void);

    /**
     * Add a service declaration to the local attribute server table.
     *
     * This functions inserts a service declaration in the attribute table
     * followed by the characteristic declarations (including characteristic
     * descriptors) present in @p service.
     *
     * The process assigns a unique attribute handle to all the elements added
     * into the attribute table. This handle is an ID that must be used for
     * subsequent interractions with the elements.
     *
     * @note There is no mirror function that removes a single service.
     * Application code can remove all the registered services by calling
     * reset().
     *
     * @attention Service, characteristics and descriptors objects registered
     * within the GattServer must remain reachable until reset() is called.
     *
     * @param[in] service The service to be added; attribute handle of services,
     * characteristic and characteristic descriptors are updated by the
     * process.
     *
     * @return BLE_ERROR_NONE if the service was successfully added.
     */
    ble_error_t addService(GattService &service);

    /**
     * Read the value of an attribute present in the local GATT server.
     *
     * @param[in] attributeHandle Handle of the attribute to read.
     * @param[out] buffer A buffer to hold the value being read.
     * @param[in,out] lengthP Length of the buffer being supplied. If the
     * attribute value is longer than the size of the supplied buffer, this
     * variable holds upon return the total attribute value length (excluding
     * offset). The application may use this information to allocate a suitable
     * buffer size.
     *
     * @return BLE_ERROR_NONE if a value was read successfully into the buffer.
     *
     * @attention read(ble::connection_handle_t, GattAttribute::Handle_t, uint8_t *, uint16_t *)
     * must be used to read Client Characteristic Configuration Descriptor (CCCD)
     * because the value of this type of attribute depends on the connection.
     */
    ble_error_t read(
        GattAttribute::Handle_t attributeHandle,
        uint8_t buffer[],
        uint16_t *lengthP
    );

    /**
     * Read the value of an attribute present in the local GATT server.
     *
     * The connection handle allows application code to read the value of a
     * Client Characteristic Configuration Descriptor for a given connection.
     *
     * @param[in] connectionHandle Connection handle.
     * @param[in] attributeHandle Attribute handle for the value attribute of
     * the characteristic.
     * @param[out] buffer A buffer to hold the value being read.
     * @param[in,out] lengthP Length of the buffer being supplied. If the
     * attribute value is longer than the size of the supplied buffer, this
     * variable holds upon return the total attribute value length (excluding
     * offset). The application may use this information to allocate a suitable
     * buffer size.
     *
     * @return BLE_ERROR_NONE if a value was read successfully into the buffer.
     */
    ble_error_t read(
        ble::connection_handle_t connectionHandle,
        GattAttribute::Handle_t attributeHandle,
        uint8_t *buffer,
        uint16_t *lengthP
    );

    /**
     * Update the value of an attribute present in the local GATT server.
     *
     * @param[in] attributeHandle Handle of the attribute to write.
     * @param[in] value A pointer to a buffer holding the new value.
     * @param[in] size Size in bytes of the new value (in bytes).
     * @param[in] localOnly If this flag is false and the attribute handle
     * written is a characteristic value, then the server sends an update
     * containing the new value to all clients that have subscribed to the
     * characteristic's notifications or indications. Otherwise, the update does
     * not generate a single server initiated event.
     *
     * @return BLE_ERROR_NONE if the attribute value has been successfully
     * updated.
     */
    ble_error_t write(
        GattAttribute::Handle_t attributeHandle,
        const uint8_t *value,
        uint16_t size,
        bool localOnly = false
    );

    /**
     * Update the value of an attribute present in the local GATT server.
     *
     * The connection handle parameter allows application code to direct
     * notification or indication resulting from the update to a specific client.
     *
     * @param[in] connectionHandle Connection handle.
     * @param[in] attributeHandle Handle for the value attribute of the
     * characteristic.
     * @param[in] value A pointer to a buffer holding the new value.
     * @param[in] size Size of the new value (in bytes).
     * @param[in] localOnly If this flag is false and the attribute handle
     * written is a characteristic value, then the server sends an update
     * containing the new value to the client identified by the parameter
     * @p connectionHandle if it is subscribed to the characteristic's
     * notifications or indications. Otherwise, the update does not generate a
     * single server initiated event.
     *
     * @return BLE_ERROR_NONE if the attribute value has been successfully
     * updated.
     */
    ble_error_t write(
        ble::connection_handle_t connectionHandle,
        GattAttribute::Handle_t attributeHandle,
        const uint8_t *value,
        uint16_t size,
        bool localOnly = false
    );

    /**
     * Determine if one of the connected clients has subscribed to notifications
     * or indications of the characteristic in input.
     *
     * @param[in] characteristic The characteristic.
     * @param[out] enabledP Upon return, *enabledP is true if updates are
     * enabled for a connected client; otherwise, *enabledP is false.
     *
     * @return BLE_ERROR_NONE if the connection and handle are found. False
     * otherwise.
     */
    ble_error_t areUpdatesEnabled(
        const GattCharacteristic &characteristic,
        bool *enabledP
    );

    /**
     * Determine if an identified client has subscribed to notifications or
     * indications of a given characteristic.
     *
     * @param[in] connectionHandle The connection handle.
     * @param[in] characteristic The characteristic.
     * @param[out] enabledP Upon return, *enabledP is true if the client
     * identified by @p connectionHandle has subscribed to notifications or
     * indications of @p characteristic; otherwise, *enabledP is false.
     *
     * @return BLE_ERROR_NONE if the connection and handle are found. False
     * otherwise.
     */
    ble_error_t areUpdatesEnabled(
        ble::connection_handle_t connectionHandle,
        const GattCharacteristic &characteristic,
        bool *enabledP
    );


    /**
     * @see ::GattServer::getPreferredConnectionParams
     */
    Gap::PreferredConnectionParams_t getPreferredConnectionParams();

    /**
     * @see ::GattServer::setPreferredConnectionParams
     */
    void setPreferredConnectionParams(const Gap::PreferredConnectionParams_t& params);

    /**
     * Indicate if the underlying stack emit events when an attribute is read by
     * a client.
     *
     * @attention This function should be overridden to return true if
     * applicable.
     *
     * @return true if onDataRead is supported; false otherwise.
     */
    bool isOnDataReadAvailable() const;

    /**
     * Add an event handler that monitors emission of characteristic value
     * updates.
     *
     * @param[in] callback Event handler being registered.
     *
     * @note It is possible to chain together multiple onDataSent callbacks
     * (potentially from different modules of an application).
     */
    void onDataSent(const DataSentCallback_t &callback);

    /**
     * Add an event handler that monitors emission of characteristic value
     * updates.
     *
     * @param[in] objPtr Pointer to the instance that is used to invoke the
     * event handler.
     * @param[in] memberPtr Event handler being registered. It is a member
     * function.
     */
    template <typename T>
    void onDataSent(T *objPtr, void (T::*memberPtr)(unsigned count));

    /**
     * Access the callchain of data sent event handlers.
     *
     * @return A reference to the DATA_SENT event callback chain.
     */
    DataSentCallbackChain_t &onDataSent();

    /**
     * Set an event handler that is called after
     * a connected peer has written an attribute.
     *
     * @param[in] callback The event handler being registered.
     *
     * @attention It is possible to set multiple event handlers. Registered
     * handlers may be removed with onDataWritten().detach(callback).
     */
    void onDataWritten(const DataWrittenCallback_t &callback);

    /**
     * Set an event handler that is called after
     * a connected peer has written an attribute.
     *
     * @param[in] objPtr Pointer to the instance that is used to invoke the
     * event handler (@p memberPtr).
     * @param[in] memberPtr Event handler being registered. It is a member
     * function.
     */
    template <typename T>
    void onDataWritten(
        T *objPtr,
        void (T::*memberPtr)(const GattWriteCallbackParams *context)
    );

    /**
     * Access the callchain of data written event handlers.
     *
     * @return A reference to the data written event callbacks chain.
     *
     * @note It is possible to register callbacks using
     * onDataWritten().add(callback).
     *
     * @note It is possible to unregister callbacks using
     * onDataWritten().detach(callback).
     */
    DataWrittenCallbackChain_t &onDataWritten();

    /**
     * Set an event handler that monitors attribute reads from connected clients.
     *
     * @param[in] callback Event handler being registered.
     *
     * @return BLE_ERROR_NOT_IMPLEMENTED if this functionality isn't available;
     * else BLE_ERROR_NONE.
     *
     * @note  This functionality may not be available on all underlying stacks.
     * Application code may work around that limitation by monitoring read
     * requests instead of read events.
     *
     * @see GattCharacteristic::setReadAuthorizationCallback()
     * @see isOnDataReadAvailable().
     *
     * @attention It is possible to set multiple event handlers. Registered
     * handlers may be removed with onDataRead().detach(callback).
     */
    ble_error_t onDataRead(const DataReadCallback_t &callback);

    /**
     * Set an event handler that monitors attribute reads from connected clients.
     *
     * @param[in] objPtr Pointer to the instance that is used to invoke the
     * event handler (@p memberPtr).
     * @param[in] memberPtr Event handler being registered. It is a member
     * function.
     */
    template <typename T>
    ble_error_t onDataRead(
        T *objPtr,
        void (T::*memberPtr)(const GattReadCallbackParams *context)
    );

    /**
     * Access the callchain of data read event handlers.
     *
     * @return A reference to the data read event callbacks chain.
     *
     * @note It is possible to register callbacks using
     * onDataRead().add(callback).
     *
     * @note It is possible to unregister callbacks using
     * onDataRead().detach(callback).
     */
    DataReadCallbackChain_t &onDataRead();

    /**
     * Set an event handler that monitors shutdown or reset of the GattServer.
     *
     * The event handler is invoked when the GattServer instance is about
     * to be shut down. This can result in a call to reset() or BLE::reset().
     *
     * @param[in] callback Event handler being registered.
     *
     * @note It is possible to set up multiple shutdown event handlers.
     *
     * @note It is possible to unregister a callback using
     * onShutdown().detach(callback)
     */
    void onShutdown(const GattServerShutdownCallback_t &callback);

    /**
     * Set an event handler that monitors shutdown or reset of the GattServer.
     *
     * The event handler is invoked when the GattServer instance is about
     * to be shut down. This can result of a call to reset() or BLE::reset().
     *
     * @param[in] objPtr Pointer to the instance that is used to invoke the
     * event handler (@p memberPtr).
     * @param[in] memberPtr Event handler being registered. It is a member
     * function.
     */
    template <typename T>
    void onShutdown(T *objPtr, void (T::*memberPtr)(const GattServer *));

    /**
     * Access the callchain of shutdown event handlers.
     *
     * @return A reference to the shutdown event callbacks chain.
     *
     * @note It is possible to register callbacks using
     * onShutdown().add(callback).
     *
     * @note It is possible to unregister callbacks using
     * onShutdown().detach(callback).
     */
    GattServerShutdownCallbackChain_t& onShutdown();

    /**
     * Set up an event handler that monitors subscription to characteristic
     * updates.
     *
     * @param[in] callback Event handler being registered.
     */
    void onUpdatesEnabled(EventCallback_t callback);

    /**
     * Set up an event handler that monitors unsubscription from characteristic
     * updates.
     *
     * @param[in] callback Event handler being registered.
     */
    void onUpdatesDisabled(EventCallback_t callback);

    /**
     * Set up an event handler that monitors notification acknowledgment.
     *
     * The event handler is called when a client sends a confirmation that it has
     * correctly received a notification from the server.
     *
     * @param[in] callback Event handler being registered.
     */
    void onConfirmationReceived(EventCallback_t callback);

    /* Entry points for the underlying stack to report events back to the user. */
protected:
    /**
     * Helper function that notifies all registered handlers of an occurrence
     * of a data written event.
     *
     * @attention Vendor implementation must invoke this function after one of
     * the GattServer attributes has been written.
     *
     * @param[in] params The data written parameters passed to the registered
     * handlers.
     */
    void handleDataWrittenEvent(const GattWriteCallbackParams *params);

    /**
     * Helper function that notifies all registered handlers of an occurrence
     * of a data read event.
     *
     * @attention Vendor implementation must invoke this function after one of
     * the GattServer attributes has been read.
     *
     * @param[in] params The data read parameters passed to the registered
     * handlers.
     */
    void handleDataReadEvent(const GattReadCallbackParams *params);

    /**
     * Helper function that notifies the registered handler of an occurrence
     * of updates enabled, updates disabled or confirmation received events.
     *
     * @attention Vendor implementation must invoke this function when a client
     * subscribes to characteristic updates, unsubscribes from characteristic
     * updates or a notification confirmation has been received.
     *
     * @param[in] type The type of event that occurred.
     * @param[in] attributeHandle The handle of the attribute concerned by the
     * event.
     */
    void handleEvent(
        GattServerEvents::gattEvent_e type,
        GattAttribute::Handle_t attributeHandle
    );

    /**
     * Helper function that notifies all registered handlers of an occurrence
     * of a data sent event.
     *
     * @attention Vendor implementation must invoke this function after the
     * emission of a notification or an indication.
     *
     * @param[in] count Number of packets sent.
     */
    void handleDataSentEvent(unsigned count);

#if !defined(DOXYGEN_ONLY)

    /* ===================================================================== */
    /*                    private implementation follows                     */

#if 0 // Disabled until reworked and reintroduced to GattServer API
public:
    /**
     * @see ble::GattServer::setDeviceName
     */
    ble_error_t setDeviceName(const uint8_t *deviceName);

    /**
     * @see ble::GattServer::getDeviceName
     */
    void getDeviceName(const uint8_t*& name, uint16_t& length);

    /**
     * @see ble::GattServer::setAppearance
     */
    void setAppearance(GapAdvertisingData::Appearance appearance);

    /**
     * @see ble::GattServer::getAppearance
     */
    GapAdvertisingData::Appearance getAppearance();

#endif // Disabled until reworked and reintroduced to GattServer API

public:
    /**
     * Return the singleton of the Cordio implementation of ble::GattServer.
     */
    static GattServer &getInstance();

    /**
     * Initialize the GattServer and add mandatory services (generic access and
     * generic attribute service).
     */
    void initialize();

private:
    void set_signing_event_handler(
        pal::SigningMonitorEventHandler *signing_event_handler
    );

    void add_default_services();

    static uint16_t compute_attributes_count(GattService& service);

    void insert_service_attribute(
        GattService& service,
        attsAttr_t *&attribute_it
    );

    ble_error_t insert_characteristic(
        GattCharacteristic *characteristic,
        attsAttr_t *&attribute_it
    );

    bool is_characteristic_valid(GattCharacteristic *characteristic);

    void insert_characteristic_declaration_attribute(
        GattCharacteristic *characteristic,
        attsAttr_t *&attribute_it
    );

    ble_error_t insert_characteristic_value_attribute(
        GattCharacteristic *characteristic,
        attsAttr_t *&attribute_it
    );

    ble_error_t insert_descriptor(
        GattCharacteristic *characteristic,
        GattAttribute* descriptor,
        attsAttr_t *&attribute_it,
        bool& cccd_created
    );

    ble_error_t insert_cccd(
        GattCharacteristic *characteristic,
        attsAttr_t *&attribute_it
    );

    static void cccd_cb(attsCccEvt_t *pEvt);
    static void att_cb(const attEvt_t *pEvt);
    static uint8_t atts_read_cb(dmConnId_t connId, uint16_t handle, uint8_t operation, uint16_t offset, attsAttr_t *pAttr);
    static uint8_t atts_write_cb(dmConnId_t connId, uint16_t handle, uint8_t operation, uint16_t offset, uint16_t len, uint8_t *pValue, attsAttr_t *pAttr);
    static uint8_t atts_auth_cb(dmConnId_t connId, uint8_t permit, uint16_t handle);
    void add_generic_access_service();
    void add_generic_attribute_service();
    void* alloc_block(size_t block_size);
    GattCharacteristic* get_auth_char(uint16_t value_handle);
    bool get_cccd_index_by_cccd_handle(GattAttribute::Handle_t cccd_handle, uint8_t& idx) const;
    bool get_cccd_index_by_value_handle(GattAttribute::Handle_t char_handle, uint8_t& idx) const;
    bool is_update_authorized(connection_handle_t connection, GattAttribute::Handle_t value_handle);

    struct alloc_block_t {
        alloc_block_t* next;
        uint8_t data[1];
    };

    struct internal_service_t {
        attsGroup_t attGroup;
        internal_service_t *next;
    };

private:
    /**
     * Event handler provided by the application.
     */
    EventHandler *eventHandler;

    /**
     * The total number of services added to the ATT table.
     */
    uint8_t serviceCount;

    /**
     * The total number of characteristics added to the ATT table.
     */
    uint8_t characteristicCount;

    /**
     * Callchain containing all registered callback handlers for data sent
     * events.
     */
    DataSentCallbackChain_t dataSentCallChain;

    /**
     * Callchain containing all registered callback handlers for data written
     * events.
     */
    DataWrittenCallbackChain_t dataWrittenCallChain;

    /**
     * Callchain containing all registered callback handlers for data read
     * events.
     */
    DataReadCallbackChain_t dataReadCallChain;

    /**
     * Callchain containing all registered callback handlers for shutdown
     * events.
     */
    GattServerShutdownCallbackChain_t shutdownCallChain;

    /**
     * The registered callback handler for updates enabled events.
     */
    EventCallback_t updatesEnabledCallback;

    /**
     * The registered callback handler for updates disabled events.
     */
    EventCallback_t updatesDisabledCallback;

    /**
     * The registered callback handler for confirmation received events.
     */
    EventCallback_t confirmationReceivedCallback;

    pal::SigningMonitorEventHandler *_signing_event_handler;

    attsCccSet_t cccds[MAX_CCCD_CNT];
    uint16_t cccd_values[MAX_CCCD_CNT];
    uint16_t cccd_handles[MAX_CCCD_CNT];
    uint8_t cccd_cnt;

    GattCharacteristic *_auth_char[MAX_CHARACTERISTIC_AUTHORIZATION_CNT];
    uint8_t _auth_char_count;

    struct {
        attsGroup_t service;
        attsAttr_t attributes[7];
        uint8_t device_name_declaration_value[5];
        uint16_t device_name_length;
        uint8_t appearance_declaration_value[5];
        uint16_t appearance;
        uint8_t ppcp_declaration_value[5];
        uint8_t ppcp[8];

        uint8_t*& device_name_value() {
            return attributes[2].pValue;
        }
    } generic_access_service;

    struct {
        attsGroup_t service;
        attsAttr_t attributes[4];
        uint8_t service_changed_declaration[5];
    } generic_attribute_service;

    internal_service_t* registered_service;
    alloc_block_t* allocated_blocks;

    uint16_t currentHandle;

    bool default_services_added;

private:
    GattServer();

    EventHandler* getEventHandler();

    GattServer(const GattServer &);
    const GattServer& operator=(const GattServer &);

#endif // !defined(DOXYGEN_ONLY)
};

/**
 * @}
 * @}
 * @}
 */

} // ble


#endif /* ifndef MBED_CORDIO_GATT_SERVER_H__ */
