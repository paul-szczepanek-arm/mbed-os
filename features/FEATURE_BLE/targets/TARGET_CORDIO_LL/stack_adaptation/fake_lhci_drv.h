/* Copyright (c) 2009-2019 Arm Limited
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef FAKE_LHCI_DRV_H_
#define FAKE_LHCI_DRV_H_

#ifdef __cplusplus
extern "C" {
#endif

uint16_t FakeChciTrRead(uint8_t prot, uint8_t type, uint16_t len, uint8_t *pData);

uint16_t FakeChciTrWrite(uint8_t prot, uint8_t type, uint16_t len, uint8_t *pData);

#ifdef __cplusplus
};
#endif

#endif /* FAKE_LHCI_DRV_H_ */
