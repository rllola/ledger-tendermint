/** ******************************************************************************
 *  (c) 2020 Zondax GmbH
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
 ******************************************************************************* */

import jest, {expect} from "jest";
import Zemu from "@zondax/zemu";
import TendermintApp from "@zondax/ledger-tendermint";

const crypto = require('crypto');
const ed25519 = require("ed25519-supercop");

const Resolve = require("path").resolve;
const APP_PATH = Resolve("../app/bin/app.elf");

const APP_SEED = "equip will roof matter pink blind book anxiety banner elbow sun young"
const sim_options = {
    logging: true,
    start_delay: 3000,
    custom: `-s "${APP_SEED}"`
    , X11: true
};

// Tendermint coin index is the same as Cosmos
const COIN = '118'

jest.setTimeout(25000)

describe('Standard', function () {
    it('can start and stop container', async function () {
        const sim = new Zemu(APP_PATH);
        try {
            await sim.start(sim_options);
        } finally {
            await sim.close();
        }
    });

    it('app version', async function () {
        const sim = new Zemu(APP_PATH);
        try {
            await sim.start(sim_options);
            const app = new TendermintApp(sim.getTransport());
            await Zemu.sleep(2000)
            const resp = await app.getVersion();

            console.log(resp);

            expect(resp.returnCode).toEqual(0x9000);
            expect(resp.errorMessage).toEqual("No errors");
            expect(resp).toHaveProperty("testMode");
            expect(resp).toHaveProperty("major");
            expect(resp).toHaveProperty("minor");
            expect(resp).toHaveProperty("patch");
        } finally {
            await sim.close();
        }
    });

    it('get address', async function () {
        const sim = new Zemu(APP_PATH);
        try {
            await sim.start(sim_options);
            const app = new TendermintApp(sim.getTransport());

            const path = `m/44'/${COIN}'/5'/0/3`;
            const resp = await app.getAddressAndPubKey(path);

            console.log(resp)

            expect(resp.returnCode).toEqual(0x9000);
            expect(resp.errorMessage).toEqual("No errors");

            // REVIEW: why it returning an empty string as an address ?
            const expected_address_string = '';
            const expected_pk = '17483e0883cf71e2fe4e12f42d1448d06f4274a73b9b6f560c5ed01a32745276';

            expect(resp.address).toEqual(expected_address_string);
            expect(resp.publicKey.toString('hex')).toEqual(expected_pk);

        } finally {
            await sim.close();
        }
    });

    it('sign basic & verify', async function () {
        const sim = new Zemu(APP_PATH);
        try {
            await sim.start(sim_options);
            const app = new TendermintApp(sim.getTransport());

            // Got from here : https://github.com/Zondax/ledger-tendermint-rs/blob/2b984728168ac98b86c07aa9ef7977fae616e2e4/src/signer.rs#L111-L137
            const path = `m/44'/${COIN}'/0'/0/1`;
            const txBlob = Buffer.from(
                "210801110100000000000000190100000000000000220b088092b8c398feffffff01",
                "hex",
            );
            
            const pkResponse = await app.getAddressAndPubKey(path);
            console.log(pkResponse)

            await Zemu.sleep(2000);

            // do not wait here..
            const signatureRequestInit = app.sign(path, txBlob);
            // Wait until we are not in the main menu
            //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
            await Zemu.sleep(2000);

            await sim.clickRight();

            let respInit = await signatureRequestInit;
            console.log(respInit);
            
            const txBlob2 = Buffer.from(
              "210801111000000000000000190200000000000000220b088092b8c398feffffff01",
              "hex",
            );

            expect(respInit.returnCode).toEqual(0x9000);
            expect(respInit.errorMessage).toEqual("No errors");
            
            const signatureRequest = app.sign(path, txBlob2);
            // Wait until we are not in the main menu
            await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());

            //await sim.clickRight();

            let resp = await signatureRequest;
            console.log(resp);
            

            // Verify signature
            const digest = crypto.createHash('sha512').update(txBlob2).digest();
            const signatureOk = ed25519.verify(resp.signatureCompact, digest, pkResponse.publicKey);
            expect(signatureOk).toEqual(true);
        } finally {
            await sim.close();
        }
    });
});
