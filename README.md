# ReweCoin Testnet

ReweCoin is a Zcash fork running its own independent testnet, actively mining.

- Current height: 605+ (growing continuously, active mining)
- Client: `/ReweCoin:1.0.0/`
- Public seed node: `49.13.62.211:28333`
- Repo: https://github.com/00impera/rewecoin-testnet

## How to connect

### 1. Clone and build

git clone https://github.com/00impera/rewecoin-testnet.git
cd rewecoin-testnet
./zcutil/build.sh -j$(nproc)

The build takes 15-30 minutes depending on your machine's resources.

### 2. Create your own datadir and config

mkdir -p ~/.rewetestnet

Create the file ~/.rewetestnet/zcash.conf with this content:

testnet=1
server=1
listen=1
rpcport=18233
port=28333
rpcuser=CHOOSE_A_USERNAME
rpcpassword=CHOOSE_A_LONG_UNIQUE_PASSWORD
addnode=49.13.62.211:28333
gen=0

Important: rpcuser and rpcpassword are local, only for accessing your own node. Do not share them with anyone and do not use the example values.

### 3. Start the node

./src/rewecoind -testnet -datadir=$HOME/.rewetestnet -printtoconsole

The node will connect to the public seed and start syncing the chain.

### 4. Check sync progress

In another terminal:

./src/rewecoin-cli -testnet -datadir=$HOME/.rewetestnet getblockcount
./src/rewecoin-cli -testnet -datadir=$HOME/.rewetestnet getpeerinfo

The height should keep increasing until it catches up with the seed.

### 5. (Optional) Mine locally

./src/rewecoin-cli -testnet -datadir=$HOME/.rewetestnet getnewaddress

Add the generated address to zcash.conf:

mineraddress=GENERATED_ADDRESS
gen=1
genproclimit=1

Restart the node.

## Known limitations

- All consensus upgrades (Overwinter, Sapling, Blossom, Heartwood, Canopy, NU5) are active from height 1. It is not possible to test the actual transition between them, only their combined behavior.
- Mined coinbase requires 100 blocks of maturity before it can be spent.
- The testnet runs on a single known seed node. Resilience to network partitioning has not been tested.

## Reporting issues

Open an issue on this repo, including relevant output from journalctl or -printtoconsole.
