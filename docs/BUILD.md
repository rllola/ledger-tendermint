#Tendermint/Cosmos Apps for Ledger Nano S

## Get source
Apart from cloning, be sure you get all the submodules, by calling:
```
git submodule update --init --recursive
```

## Dependencies

#### Ledger Nano S

This project requires ledger firmware 1.5.5

The current repository keeps track of Ledger's SDK but it is posible to override it by changing the git submodule.

#### CircleCI CLI

CircleCI allows compiling `BOLOS` firmware both in Linux and MacOS. The CLI will download a docker container ready to run.

To install, follow the instructions here:

https://circleci.com/docs/2.0/local-cli/#installing-the-circleci-local-cli-on-macos-and-linux-distros

#### Docker CE

CircleCI CLI should have instructed you to install Docker. Just in case, you can find instructions here too:

https://docs.docker.com/install/


#### OS Dependencies

**Ubuntu**

Install the following packages:
```
sudo apt-get update && sudo apt-get -y install build-essential git sudo wget cmake libssl-dev libgmp-dev autoconf libtool python-pip
```

**OSX**

It is recommended that you install brew and xcode.

Additionally you will need to:

```
brew install libusb
```

#### Python Tools

To avoid pollution your host, it is recommended that you create a virtual environment. If you are using [Anaconda](https://www.anaconda.com/distribution/#download-section) (our preference), you can do something like:

```shell script
create -n ledger python=3.7
conda activate ledger
```

Then you can safely install all the python dependencies in that enviroment. Ledger firmware 1.5.5 requires ledgerblue >= 0.1.21.

In most cases, `make deps` should be able to install everything:

```bash
make deps
```

# Building
There are different local builds:

 - Generic C++ code and run unit tests
 - BOLOS firmware

Please refer to the [Ledger-Cosmos](https://github.com/cosmos/ledger-cosmos) for the complete source code, build instructions, etc (unit tests, integration tests, documentation, etc.)


Up to date instructions are kept [here](https://github.com/cosmos/ledger-cosmos/blob/master/docs/BUILD.md)
