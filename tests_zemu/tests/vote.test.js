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

describe('Advance', function () {
  it('send vote 1, accept, send vote 2 (height+1), success', async function () {
    const sim = new Zemu(APP_PATH);
    try {
      await sim.start(sim_options);
      const app = new TendermintApp(sim.getTransport());
      await Zemu.sleep(2000)
      
      const path = `m/44'/${COIN}'/0'/0/1`;
      const txBlob = Buffer.from(
        "210801110100000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 1
        "hex",
      );
      
      // do not wait here..
      const signatureRequestInit = app.sign(path, txBlob);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);
      await sim.clickRight();
      
      let respInit = await signatureRequestInit;
      console.log(respInit);
      
      expect(respInit.returnCode).toEqual(0x9000);
      expect(respInit.errorMessage).toEqual("No errors");
      
      const txBlob2 = Buffer.from(
        "210801110200000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 2
        "hex",
      );
    
      const signatureRequest = app.sign(path, txBlob2);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);

      let resp = await signatureRequest;
      console.log(resp);
      
      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");
    } finally {
      await sim.close();
    }
  });
  
  it('send vote 1, accept, send vote 2 (height-1), fail', async function () {
    const sim = new Zemu(APP_PATH);
    try {
      await sim.start(sim_options);
      const app = new TendermintApp(sim.getTransport());
      await Zemu.sleep(2000)
      
      const path = `m/44'/${COIN}'/0'/0/1`;
      const txBlob = Buffer.from(
        "210801110200000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 2
        "hex",
      );
      
      // do not wait here..
      const signatureRequestInit = app.sign(path, txBlob);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);
      await sim.clickRight();
      
      let respInit = await signatureRequestInit;
      console.log(respInit);
      
      expect(respInit.returnCode).toEqual(0x9000);
      expect(respInit.errorMessage).toEqual("No errors");
      
      const txBlob2 = Buffer.from(
        "210801110100000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 1
        "hex",
      );
    
      const signatureRequest = app.sign(path, txBlob2);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);

      let resp = await signatureRequest;
      console.log(resp);
      
      expect(resp.returnCode).toEqual(0x6985);
      expect(resp.errorMessage).toEqual("Conditions not satisfied");
    } finally {
      await sim.close();
    }
  });
  
  it('send vote 1, reject, send 2 vote it should ask in the screen again', async function () {
    const sim = new Zemu(APP_PATH);
    try {
      await sim.start(sim_options);
      const app = new TendermintApp(sim.getTransport());
      await Zemu.sleep(2000)
      
      const path = `m/44'/${COIN}'/0'/0/1`;
      const txBlob = Buffer.from(
        "210801110100000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 1
        "hex",
      );
      
      // do not wait here..
      const signatureRequestInit = app.sign(path, txBlob);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);
      await sim.clickLeft();
      
      let respInit = await signatureRequestInit;
      console.log(respInit);
      
      expect(respInit.returnCode).toEqual(0x6986);
      expect(respInit.errorMessage).toEqual("Transaction rejected");
    
      const signatureRequest = app.sign(path, txBlob);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);
      await sim.clickRight();

      let resp = await signatureRequest;
      console.log(resp);
      
      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");
    } finally {
      await sim.close();
    }
  });

  it('send vote 1, accept, send vote 2 (valid so sign), send vote 3 (valid so sign)', async function () {
    const sim = new Zemu(APP_PATH);
    try {
      await sim.start(sim_options);
      const app = new TendermintApp(sim.getTransport());
      await Zemu.sleep(2000)
      
      const path = `m/44'/${COIN}'/0'/0/1`;
      const txBlob = Buffer.from(
        "210801110100000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 1
        "hex",
      );
      
      // do not wait here..
      const signatureRequestInit = app.sign(path, txBlob);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);
      await sim.clickRight();
      
      let respInit = await signatureRequestInit;
      console.log(respInit);
      
      expect(respInit.returnCode).toEqual(0x9000);
      expect(respInit.errorMessage).toEqual("No errors");
      
      const txBlob2 = Buffer.from(
        "210801110200000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 2
        "hex",
      );
    
      const signatureRequest = app.sign(path, txBlob2);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);

      let resp = await signatureRequest;
      console.log(resp);
      
      expect(resp.returnCode).toEqual(0x9000);
      expect(resp.errorMessage).toEqual("No errors");
      
      const txBlob3 = Buffer.from(
        "210801110300000000000000190100000000000000220b088092b8c398feffffff01", // round 1 height 3
        "hex",
      );
    
      const signatureRequestBis = app.sign(path, txBlob3);
      // Wait until we are not in the main menu
      //await sim.waitUntilScreenIsNot(sim.getMainMenuSnapshot());
      await Zemu.sleep(2000);

      let respBis = await signatureRequestBis;
      console.log(respBis);
      
      expect(respBis.returnCode).toEqual(0x9000);
      expect(respBis.errorMessage).toEqual("No errors");
    } finally {
      await sim.close();
    }
  });
});
