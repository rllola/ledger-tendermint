/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018 ZondaX GmbH
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

#include "app_main.h"
#include "view.h"
#include "lib/vote_buffer.h"
#include "lib/vote_fsm.h"
#include "signature.h"

#include <os_io_seproxyhal.h>
#include <os.h>
#include <string.h>

unsigned char public_key[32];
cx_ecfp_private_key_t cx_privateKey;
uint8_t keys_initialized = 0;

unsigned char G_io_seproxyhal_spi_buffer[IO_SEPROXYHAL_BUFFER_SIZE_B];

unsigned char io_event(unsigned char channel) {
    switch (G_io_seproxyhal_spi_buffer[0]) {
        case SEPROXYHAL_TAG_FINGER_EVENT: //
            UX_FINGER_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_BUTTON_PUSH_EVENT: // for Nano S
            UX_BUTTON_PUSH_EVENT(G_io_seproxyhal_spi_buffer);
            break;

        case SEPROXYHAL_TAG_DISPLAY_PROCESSED_EVENT:
            if (!UX_DISPLAYED())
                UX_DISPLAYED_EVENT();
            break;

        case SEPROXYHAL_TAG_TICKER_EVENT: { //
            UX_TICKER_EVENT(G_io_seproxyhal_spi_buffer, {
                    if (UX_ALLOWED) {
                        UX_REDISPLAY();
                    }
            });
            break;
        }

            // unknown events are acknowledged
        default:
            UX_DEFAULT_EVENT();
            break;
    }
    if (!io_seproxyhal_spi_is_status_sent()) {
        io_seproxyhal_general_status();
    }
    return 1; // DO NOT reset the current APDU transport
}

unsigned short io_exchange_al(unsigned char channel, unsigned short tx_len) {
    switch (channel & ~(IO_FLAGS)) {
        case CHANNEL_KEYBOARD:
            break;

            // multiplexed io exchange over a SPI channel and TLV encapsulated protocol
        case CHANNEL_SPI:
            if (tx_len) {
                io_seproxyhal_spi_send(G_io_apdu_buffer, tx_len);

                if (channel & IO_RESET_AFTER_REPLIED) {
                    reset();
                }
                return 0; // nothing received from the master so far (it's a tx
                // transaction)
            } else {
                return io_seproxyhal_spi_recv(G_io_apdu_buffer, sizeof(G_io_apdu_buffer), 0);
            }

        default:
            THROW(INVALID_PARAMETER);
    }
    return 0;
}

void app_init() {
    io_seproxyhal_init();
    USB_power(0);
    USB_power(1);
    view_display_main_menu();
}

bool extractBip32(uint8_t *depth, uint32_t path[10], uint32_t rx, uint32_t offset) {
    if (rx < offset + 1) {
        return 0;
    }

    *depth = G_io_apdu_buffer[offset];
    const uint16_t req_offset = 4 * *depth + 1 + offset;

    if (rx < req_offset || *depth > 10) {
        return 0;
    }

    memcpy(path, G_io_apdu_buffer + offset + 1, *depth * 4);
    return 1;
}

bool process_chunk(volatile uint32_t *tx, uint32_t rx) {
    int packageIndex = G_io_apdu_buffer[OFFSET_PCK_INDEX];
    int packageCount = G_io_apdu_buffer[OFFSET_PCK_COUNT];

    uint16_t offset = OFFSET_DATA;
    if (rx < offset) {
        THROW(APDU_CODE_DATA_INVALID);
    }

    if (packageIndex == 1) {
        vote_initialize();
        vote_reset();
    }

    if (vote_append(G_io_apdu_buffer + offset, rx - offset) != rx - offset) {
        THROW(APDU_CODE_OUTPUT_BUFFER_TOO_SMALL);
    }

    return packageIndex == packageCount;
}

void format_pubkey(unsigned char *outputBuffer, cx_ecfp_public_key_t *pubKey) {
    for (int i = 0; i < 32; i++) {
        outputBuffer[i] = pubKey->W[64 - i];
    }

    if ((pubKey->W[32] & 1) != 0) {
        outputBuffer[31] |= 0x80;
    }
}

void extract_keys(uint8_t bip32_depth, uint32_t bip32_path[10]) {
    cx_ecfp_public_key_t cx_publicKey;
    uint8_t privateKeyData[32];

    // Generate keys
    os_perso_derive_node_bip32(CX_CURVE_Ed25519,
                               bip32_path,
                               bip32_depth,
                               privateKeyData, NULL);

    keys_ed25519(&cx_publicKey, &cx_privateKey, privateKeyData);
    memset(privateKeyData, 0, 32);

    format_pubkey(public_key, &cx_publicKey);
    keys_initialized = 1;
}

unsigned int sign_vote() {
    uint8_t *signature = G_io_apdu_buffer;
    unsigned int signature_capacity = IO_APDU_BUFFER_SIZE - 2;
    unsigned int info = 0;

    return cx_eddsa_sign(&cx_privateKey,
                         CX_LAST,
                         CX_SHA512,
                         vote_get_buffer(),
                         vote_get_buffer_length(),
                         NULL,
                         0,
                         signature,
                         signature_capacity,
                         &info);
}

void handleApdu(volatile uint32_t *flags, volatile uint32_t *tx, uint32_t rx) {
    uint16_t sw = 0;

    BEGIN_TRY
    {
        TRY
        {
            if (G_io_apdu_buffer[OFFSET_CLA] != CLA) {
                THROW(APDU_CODE_CLA_NOT_SUPPORTED);
            }

            if (rx < 5) {
                THROW(APDU_CODE_WRONG_LENGTH);
            }

            switch (G_io_apdu_buffer[OFFSET_INS]) {
                case INS_GET_VERSION: {
#ifdef TESTING_ENABLED
                    G_io_apdu_buffer[0] = 0xFF;
#else
                    G_io_apdu_buffer[0] = 0;
#endif
                    G_io_apdu_buffer[1] = LEDGER_MAJOR_VERSION;
                    G_io_apdu_buffer[2] = LEDGER_MINOR_VERSION;
                    G_io_apdu_buffer[3] = LEDGER_PATCH_VERSION;
                    *tx += 4;
                    THROW(APDU_CODE_OK);
                    break;
                }

                case INS_PUBLIC_KEY_ED25519: {
                    uint8_t bip32_depth;
                    uint32_t bip32_path[10];

                    if (!extractBip32(&bip32_depth, bip32_path, rx, OFFSET_DATA)) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    extract_keys(bip32_depth, bip32_path);

                    os_memmove(G_io_apdu_buffer, public_key, sizeof(public_key));
                    *tx += sizeof(public_key);

                    THROW(APDU_CODE_OK);
                }

                case INS_SIGN_ED25519: {
                    if (!keys_initialized) {
                        THROW(APDU_CODE_COMMAND_NOT_ALLOWED);
                    }

                    if (!process_chunk(tx, rx)) {
                        THROW(APDU_CODE_OK);
                    }

                    parse_error_t error_code = vote_parse();

                    if (error_code != parse_ok) {
                        G_io_apdu_buffer[*tx] = (uint8_t) error_code;
                        *tx++;
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    vote_t *vote = vote_get();
                    vote_state_t *vote_state = vote_state_get();

                    if (vote == NULL || vote_state == NULL) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    if (!vote_state->isInitialized) {
                        // Show values and ask user before setting state
                        view_set_msg(vote);
                        view_display_vote_init();
                        *flags |= IO_ASYNCH_REPLY;
                        break;
                    }

                    // Check with vote FSM if vote can be signed
                    if (!try_state_transition()) {
                        THROW(APDU_CODE_DATA_INVALID);
                    }

                    *tx = sign_vote();
                    view_set_state(vote_state, public_key);
                    THROW(APDU_CODE_OK);

                }
                    break;

                default:
                    THROW(APDU_CODE_INS_NOT_SUPPORTED);
            }
        }
        CATCH(EXCEPTION_IO_RESET)
        {
            THROW(EXCEPTION_IO_RESET);
        }
        CATCH_OTHER(e)
        {
            switch (e & 0xF000) {
                case 0x6000:
                case APDU_CODE_OK:
                    sw = e;
                    break;
                default:
                    sw = 0x6800 | (e & 0x7FF);
                    break;
            }
            G_io_apdu_buffer[*tx] = sw >> 8;
            G_io_apdu_buffer[*tx + 1] = sw;
            *tx += 2;
        }
        FINALLY
        {
        }
    }
    END_TRY;
}

void reject_vote_state() {
    set_code(G_io_apdu_buffer, 0, APDU_CODE_COMMAND_NOT_ALLOWED);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, 2);
    view_display_main_menu();
}

void accept_vote_state(vote_t *v) {
    vote_state_t *s = vote_state_get();

    s->vote.Type = v->Type;
    s->vote.Height = v->Height;
    s->vote.Round = v->Round;
    s->isInitialized = 1;

    view_set_state(s, public_key);

    unsigned int tx = sign_vote();

    set_code(G_io_apdu_buffer + tx, 0, APDU_CODE_OK);
    io_exchange(CHANNEL_APDU | IO_RETURN_AFTER_TX, tx + 2);

    view_display_vote_processing();
}

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"

void app_main() {
    volatile uint32_t rx = 0, tx = 0, flags = 0;

    vote_state_reset();
    view_set_vote_reset_eh(&vote_state_reset);
    view_set_accept_eh(&accept_vote_state);
    view_set_reject_eh(&reject_vote_state);

    keys_initialized = 0;

    for (;;) {
        volatile uint16_t sw = 0;

        BEGIN_TRY;
        {
            TRY;
            {
                rx = tx;
                tx = 0;
                rx = io_exchange(CHANNEL_APDU | flags, rx);
                flags = 0;

                if (rx == 0)
                    THROW(APDU_CODE_EMPTY_BUFFER);

                handleApdu(&flags, &tx, rx);
            }
            CATCH_OTHER(e);
            {
                switch (e & 0xF000) {
                    case 0x6000:
                    case 0x9000:
                        sw = e;
                        break;
                    default:
                        sw = 0x6800 | (e & 0x7FF);
                        break;
                }
                G_io_apdu_buffer[tx] = sw >> 8;
                G_io_apdu_buffer[tx + 1] = sw;
                tx += 2;
            }
            FINALLY;
            {}
        }
        END_TRY;
    }
}

#pragma clang diagnostic pop

