/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018, 2019 Zondax GmbH
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

#include "actions.h"
#include "vote.h"
#include "crypto.h"
#include "vote_buffer.h"
#include "apdu_codes.h"
#include <zxmacros.h>

void action_reset() {
    vote_state_reset();
}

void action_reject() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
}

void action_accept() {
    vote_state.vote.Type = vote.Type;
    vote_state.vote.Height = vote.Height;
    vote_state.vote.Round = vote.Round;
    vote_state.isInitialized = 1;

    int tx = action_sign();

    set_code(G_io_apdu_buffer + tx, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx + 2);
}

int action_sign() {
    const uint8_t *message = vote_get_buffer();
    const uint32_t messageLen = vote_get_buffer_length();

    uint8_t *signature = G_io_apdu_buffer;
    unsigned int signature_capacity = IO_APDU_BUFFER_SIZE - 2;

    uint16_t signatureLength = sign_ed25519(message, messageLen, signature, signature_capacity);

    return signatureLength;
}

void actions_getkeys() {
    keys_precache();
}
