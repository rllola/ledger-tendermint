# Ledger-Tendermint
[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](https://opensource.org/licenses/Apache-2.0)
[![CircleCI](https://circleci.com/gh/ZondaX/ledger-tendermint/tree/master.svg?style=shield)](https://circleci.com/gh/ZondaX/ledger-tendermint/tree/master)

This repository contains:

  - Ledger Nano S/X Tendermint validator BOLOS app
  - Specs / Documentation 
  - C++ unit tests
  - Fuzzing scripts

Source code for apps is linked as submodules to allow for Ledger's build infrastructure.

For development purposes, this repo is recommended as it includes unit tests, tools, etc.  

## Installing

### Validator app

The validator app is available in [Ledger Live](https://www.ledger.com/pages/ledger-live). 

**The app has been released in TEST mode for now! WARNING: It is possible to exit without unplugging**

![](docs/img/cosmos_app1.png)

  - Enable developer mode (last option):

![](docs/img/cosmos_app2.png)

To install, please follow the build instructions in the following section.

  - Now go back to manager and search for Tendermint:

![](docs/img/tendermint_app.png)

## Building

The following document describes how to build the apps: [Build instructions](docs/BUILD.md)

## Specifications

**Validator App**

The target of this app are validators. The app signs votes/proposals via the Tendermint/Cosmos KMS (Key management system).

  - [APDU Protocol](https://github.com/tendermint/ledger-validator-app/blob/master/docs/APDUSPEC.md)
  - [Transaction format](https://github.com/tendermint/ledger-validator-app/blob/master/docs/TXSPEC.md)
  - [User interface](https://github.com/tendermint/ledger-validator-app/blob/master/docs/UISPEC.md)
