# ReweCoin (mainnet)

ReweCoin is a Zcash 6.0.0 fork with its own independent chain.

- Algorithm: Equihash (200,9)
- Target block spacing: 75 s (Blossom active from height 1)
- P2P port: 28233
- RPC port: 8232 (local use only, never expose it)
- Genesis block hash: `0006ef5ee4ef99c294e020bf8398bb342648ce289bf748b9b11f27f4463f6d19`
- Seeds (hardcoded): `seed1.rewecoin.com`, `seed2.rewecoin.com`
- Branch: `master` (this branch is mainnet; `main` is an unrelated Foundry/Solidity project)

## Status

Early-stage network. At the time of writing it is run by a single operator and mining difficulty is at its floor. Do not treat it as a decentralized or production chain yet. Blocks can be produced much faster than the 75 s target if hashrate appears.

## Build

Precompiled binaries are not provided. Build from source on a recent Linux (the reference build was made on Ubuntu 26.04).

    git clone https://github.com/00impera/rewecoin.git
    cd rewecoin
    ./zcutil/build.sh -j$(nproc)

The build takes 15-30 minutes.

## Run a node

    mkdir -p ~/.rewemainnet
    cat > ~/.rewemainnet/rewecoin.conf <<CONF
    server=1
    listen=1
    port=28233
    rpcport=8232
    rpcuser=CHOOSE_A_USERNAME
    rpcpassword=CHOOSE_A_LONG_UNIQUE_PASSWORD
    addnode=seed1.rewecoin.com:28233
    addnode=seed2.rewecoin.com:28233
    gen=0
    CONF

    ./src/rewecoind -datadir=$HOME/.rewemainnet -conf=$HOME/.rewemainnet/rewecoin.conf -printtoconsole

rpcuser and rpcpassword are local, only for your own node. Do not reuse the example values.

## Verify you are on the right chain

    CLI="./src/rewecoin-cli -datadir=$HOME/.rewemainnet -conf=$HOME/.rewemainnet/rewecoin.conf"
    $CLI getblockhash 0
    $CLI getblockcount
    $CLI getpeerinfo

`getblockhash 0` must return the genesis hash above. If it differs, you are not on this chain. `getblockcount` should keep increasing, and `getpeerinfo` should show at least one connection.

## Mining (optional)

Use a dedicated address, not an exchange or custodial address:

    $CLI getnewaddress

Add to rewecoin.conf, then restart:

    mineraddress=YOUR_ADDRESS
    gen=1
    genproclimit=1

Coinbase outputs need 100 blocks of maturity before they can be spent.

## Known limitations

- Single-operator network, one effective seed host.
- Difficulty floor: start with CPU mining and expect fast, irregular blocks.
- All consensus upgrades are active from height 1, so upgrade transitions cannot be tested.

## Issues

Open an issue on this repo with relevant output from `-printtoconsole` or journalctl.
