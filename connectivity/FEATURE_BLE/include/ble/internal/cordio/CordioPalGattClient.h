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

#ifndef BLE_PAL_ATTCLIENTTOGATTCLIENTADAPTER_H_
#define BLE_PAL_ATTCLIENTTOGATTCLIENTADAPTER_H_

#include "ble/internal/pal/PalGattClient.h"

namespace ble {

class PalAttClient;

/**
 * Adapt a PalAttClient into a PalGattClient.
 *
 * This class let vendors define their abstraction layer in term of an PalAttClient
 * and adapt any PalAttClient into a PalGattClient.
 */
class PalGattClient : public interface::PalGattClient {
public:
    static const uint16_t END_ATTRIBUTE_HANDLE = 0xFFFF;
    static const uint16_t SERVICE_TYPE_UUID = 0x2800;
    static const uint16_t INCLUDE_TYPE_UUID = 0x2802;
    static const uint16_t CHARACTERISTIC_TYPE_UUID = 0x2803;

    /**
     * Construct an instance of PalGattClient from an instance of PalAttClient.
     * @param client The client to adapt.
     */
    PalGattClient(PalAttClient& client);

    /**
     * @see ble::PalGattClient::exchange_mtu
     */
    ble_error_t exchange_mtu(connection_handle_t connection);

    /**
     * @see ble::PalGattClient::get_mtu_size
     */
    ble_error_t get_mtu_size(
        connection_handle_t connection_handle,
        uint16_t& mtu_size
    );

    /**
     * @see ble::PalGattClient::discover_primary_service
     */
    ble_error_t discover_primary_service(
        connection_handle_t connection,
        attribute_handle_t discovery_range_begining
    );

    /**
     * @see ble::PalGattClient::discover_primary_service_by_service_uuid
     */
    ble_error_t discover_primary_service_by_service_uuid(
        connection_handle_t connection_handle,
        attribute_handle_t discovery_range_begining,
        const UUID& uuid
    );

    /**
     * @see ble::PalGattClient::find_included_service
     */
    ble_error_t find_included_service(
        connection_handle_t connection_handle,
        attribute_handle_range_t service_range
    );

    /**
     * @see ble::PalGattClient::discover_characteristics_of_a_service
     */
    ble_error_t discover_characteristics_of_a_service(
        connection_handle_t connection_handle,
        attribute_handle_range_t discovery_range
    );

    /**
     * @see ble::PalGattClient::discover_characteristics_descriptors
     */
    ble_error_t discover_characteristics_descriptors(
        connection_handle_t connection_handle,
        attribute_handle_range_t descriptors_discovery_range
    );

    /**
     * @see ble::PalGattClient::read_attribute_value
     */
    ble_error_t read_attribute_value(
        connection_handle_t connection_handle,
        attribute_handle_t attribute_handle
    );

    /**
     * @see ble::PalGattClient::read_using_characteristic_uuid
     */
    ble_error_t read_using_characteristic_uuid(
        connection_handle_t connection_handle,
        attribute_handle_range_t read_range,
        const UUID& uuid
    );

    /**
     * @see ble::PalGattClient::read_attribute_blob
     */
    ble_error_t read_attribute_blob(
        connection_handle_t connection_handle,
        attribute_handle_t attribute_handle,
        uint16_t offset
    );

    /**
     * @see ble::PalGattClient::read_multiple_characteristic_values
     */
    ble_error_t read_multiple_characteristic_values(
        connection_handle_t connection_handle,
        const Span<const attribute_handle_t>& characteristic_value_handles
    );

    /**
     * @see ble::PalGattClient::write_without_response
     */
    ble_error_t write_without_response(
        connection_handle_t connection_handle,
        attribute_handle_t characteristic_value_handle,
        const Span<const uint8_t>& value
    );

    /**
     * @see ble::PalGattClient::signed_write_without_response
     */
    ble_error_t signed_write_without_response(
        connection_handle_t connection_handle,
        attribute_handle_t characteristic_value_handle,
        const Span<const uint8_t>& value
    );

    /**
     * @see ble::PalGattClient::write_attribute
     */
    ble_error_t write_attribute(
        connection_handle_t connection_handle,
        attribute_handle_t attribute_handle,
        const Span<const uint8_t>& value
    );

    /**
     * @see ble::PalGattClient::queue_prepare_write
     */
    ble_error_t queue_prepare_write(
        connection_handle_t connection_handle,
        attribute_handle_t characteristic_value_handle,
        const Span<const uint8_t>& value,
        uint16_t offset
    );

    /**
     * @see ble::PalGattClient::execute_write_queue
     */
    ble_error_t execute_write_queue(
        connection_handle_t connection_handle,
        bool execute
    );

    /**
     * @see ble::PalGattClient::initialize
     */
    ble_error_t initialize();

    /**
     * @see ble::PalGattClient::terminate
     */
    ble_error_t terminate();

private:
    PalAttClient& _client;
};

} // namespace ble

#endif /* BLE_PAL_ATTCLIENTTOGATTCLIENTADAPTER_H_ */
