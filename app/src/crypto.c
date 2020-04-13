/*******************************************************************************
*   (c) 2016 Ledger
*   (c) 2018 Zondax GmbH
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
#include "crypto.h"

#include "cx.h"
#include "apdu_codes.h"
#include "coin.h"

#define BIP32_DEPTH                 5

uint8_t keys_initialized = 0;

uint8_t public_key[32];
cx_ecfp_private_key_t cx_privateKey;

__Z_INLINE void format_pubkey(uint8_t *outputBuffer, cx_ecfp_public_key_t *pubKey) {
    for (int i = 0; i < 32; i++) {
        outputBuffer[i] = pubKey->W[64 - i];
    }

    if ((pubKey->W[32] & 1) != 0) {
        outputBuffer[31] |= 0x80;
    }
}

// keep cached to keep latency low
#define CLEAR_PRIVATE_KEY /*MEMZERO(&cx_privateKey, sizeof(cx_ecfp_private_key_t))*/

void keys_precache() {
    uint32_t bip32_path[BIP32_DEPTH];
    bip32_path[0] = HDPATH_0_DEFAULT;
    bip32_path[1] = HDPATH_1_DEFAULT;
    bip32_path[2] = HDPATH_2_DEFAULT;
    bip32_path[3] = HDPATH_3_DEFAULT;
    bip32_path[4] = HDPATH_4_DEFAULT;

    cx_ecfp_public_key_t cx_publicKey;
    uint8_t privateKeyData[32];

    BEGIN_TRY
    {
        TRY
        {
            // Generate keys
            os_perso_derive_node_bip32_seed_key(HDW_NORMAL, CX_CURVE_Ed25519,
                                                bip32_path, BIP32_DEPTH,
                                                privateKeyData,
                                                NULL, NULL, 0);

            cx_ecfp_init_private_key(CX_CURVE_Ed25519, privateKeyData, 32, &cx_privateKey);
            cx_ecfp_init_public_key(CX_CURVE_Ed25519, NULL, 0, &cx_publicKey);
            cx_ecfp_generate_pair(CX_CURVE_Ed25519, &cx_publicKey, &cx_privateKey, 1);
        }
        FINALLY
        {
            CLEAR_PRIVATE_KEY;
            MEMZERO(privateKeyData, 32);
        }
    }
    END_TRY;

    format_pubkey(public_key, &cx_publicKey);
    keys_initialized = 1;
}

uint16_t sign_ed25519(const uint8_t *message, unsigned int messageLength,
                      uint8_t *signature, unsigned int signatureCapacity) {
    uint8_t messageDigest[CX_SHA512_SIZE];
    uint16_t signatureLength;

    // Hash it
    cx_sha512_t ctx;
    cx_sha512_init((cx_sha512_t * ) & ctx.header);
    cx_hash(&ctx.header, CX_LAST, message, messageLength, messageDigest, CX_SHA512_SIZE);

    BEGIN_TRY
    {
        TRY
        {
            unsigned int info = 0;
            signatureLength = cx_eddsa_sign(&cx_privateKey,
                                            CX_LAST,
                                            CX_SHA512, messageDigest, CX_SHA512_SIZE,
                                            NULL, 0,
                                            signature, signatureCapacity,
                                            &info);
        }
        FINALLY
        {
            CLEAR_PRIVATE_KEY;
        }
    }
    END_TRY;

    return signatureLength;
}
