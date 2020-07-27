/* mbed Microcontroller Library
 * Copyright (c) 2017-2020 ARM Limited
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

#include "ble/internal/cordio/CordioPalGenericAccessService.h"

ble_error_t PalGenericAccessService::get_peripheral_preferred_connection_parameters(
    ble::Gap::PreferredConnectionParams_t& parameters
) {
#if BLE_FEATURE_GATT_SERVER
    parameters = gatt_server().getPreferredConnectionParams();
    return BLE_ERROR_NONE;
#else
    return BLE_ERROR_NOT_IMPLEMENTED;
#endif // BLE_FEATURE_GATT_SERVER
}

ble_error_t PalGenericAccessService::set_peripheral_preferred_connection_parameters(
    const ble::Gap::PreferredConnectionParams_t& parameters
) {
#if BLE_FEATURE_GATT_SERVER
    gatt_server().setPreferredConnectionParams(parameters);
    return BLE_ERROR_NONE;
#else
    return BLE_ERROR_NOT_IMPLEMENTED;
#endif // BLE_FEATURE_GATT_SERVER
}


#if BLE_FEATURE_GATT_SERVER
ble::GattServer& PalGenericAccessService::gatt_server() {
    return ble::GattServer::getInstance();
}
#endif // BLE_FEATURE_GATT_SERVER
