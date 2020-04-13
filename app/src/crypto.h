/*******************************************************************************
*   (c) 2019 Zondax GmbH
*
*  Licensed under the Apache License, Version 2.0 (the "License");
*  you may not use this file except in compliance with the License.
*  You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
*  Unless required by applicable law or agreed to in writing, software
*  distributed under the License is distributed on an "AS IS" BASIS,
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*  See the License for the specific language governing permissions and
*  limitations under the License.
********************************************************************************/

#pragma once

#include "os.h"

extern uint8_t public_key[32];
extern uint8_t keys_initialized;

void keys_precache();

/// sign_ed25519
/// \param message
/// \param message_length
/// \param signature
/// \param signature_capacity
/// \return size of the signature
uint16_t sign_ed25519(const uint8_t *message, unsigned int message_length,
                      uint8_t *signature, unsigned int signature_capacity);
