// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2014 The Bitcoin Core developers
// Copyright (c) 2015-2025 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or https://www.opensource.org/licenses/mit-license.php .

#include "chainparams.h"
#include "consensus/merkle.h"
#include "key_io.h"
#include "main.h"
#include "crypto/equihash.h"

#include "tinyformat.h"
#include "util/system.h"
#include "util/strencodings.h"

#include <assert.h>
#include <optional>
#include <variant>

#include <boost/assign/list_of.hpp>

#include "chainparamsseeds.h"

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    // To create a genesis block for a new chain which is Overwintered:
    //   txNew.nVersion = OVERWINTER_TX_VERSION
    //   txNew.fOverwintered = true
    //   txNew.nVersionGroupId = OVERWINTER_VERSION_GROUP_ID
    //   txNew.nExpiryHeight = <default value>
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520617983 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.nSolution = nSolution;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

/**
 * Build the genesis block. Note that the output of its generation
 * transaction cannot be spent since it did not originally exist in the
 * database (and is in any case of zero value).
 *
 * >>> from hashlib import blake2s
 * >>> 'Zcash' + blake2s(b'The Economist 2016-10-29 Known unknown: Another crypto-currency is born. BTC#436254 0000000000000000044f321997f336d2908cf8c8d6893e88dbf067e2d949487d ETH#2521903 483039a6b6bd8bd05f0584f9a078d075e454925eb71c1f13eaff59b405a721bb DJIA close on 27 Oct 2016: 18,169.68').hexdigest()
 *
 * CBlock(hash=00040fe8, ver=4, hashPrevBlock=00000000000000, hashMerkleRoot=c4eaa5, nTime=1477641360, nBits=1f07ffff, nNonce=4695, vtx=1)
 *   CTransaction(hash=c4eaa5, ver=1, vin.size=1, vout.size=1, nLockTime=0)
 *     CTxIn(COutPoint(000000, -1), coinbase 04ffff071f0104455a6361736830623963346565663862376363343137656535303031653335303039383462366665613335363833613763616331343161303433633432303634383335643334)
 *     CTxOut(nValue=0.00000000, scriptPubKey=0x5F1DF16B2B704C8A578D0B)
 *   vMerkleTree: c4eaa5
 */
static CBlock CreateGenesisBlock(uint32_t nTime, const uint256& nNonce, const std::vector<unsigned char>& nSolution, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const char* pszTimestamp = "ReweCoin - A new chain begins - Sep 2026";
    const CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nSolution, nBits, nVersion, genesisReward);
}

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */

const arith_uint256 maxUint = UintToArith256(uint256S("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"));

class CMainParams : public CChainParams {
public:
    CMainParams() {
        keyConstants.strNetworkID = "main";
        strCurrencyUnits = "ZEC";
        keyConstants.bip44CoinType = 133; // As registered in https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 4000;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0007ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = std::nullopt;
        consensus.fPowNoRetargeting = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170005;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170009;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170011;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170013;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170100;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170120;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170140;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.nTemporaryOrchardDisablingSoftForkHeight = 3363426;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nProtocolVersion = 170150;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nActivationHeight = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;

        // guarantees the first 2 characters, when base58 encoded, are "t1"
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x0D,0x84};
        // guarantees the first 2 characters, when base58 encoded, are "t3"
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x0D,0x89};
        // the first character, when base58 encoded, is "5" or "K" or "L" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0x80};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x88,0xB2,0x1E};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x88,0xAD,0xE4};
        // guarantees the first 2 characters, when base58 encoded, are "zc"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0x9A};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVK"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAB,0xD3};
        // guarantees the first 2 characters, when base58 encoded, are "SK"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAB,0x36};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "rws";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "rwviews";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "rwivks";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-rewecoin";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "rwxviews";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "tex";
        {
#if 0
            auto canopyActivation = consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight;
            auto nu6Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight;
            auto nu6_1Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight;

            // ZIP 214 Revision 0
            std::vector<std::string> bp_addresses = {
                "t3LmX1cxWPPPqL4TZHx42HU3U5ghbFjRiif",
                "t3Toxk1vJQ6UjWQ42tUJz2rV2feUWkpbTDs",
                "t3ZBdBe4iokmsjdhMuwkxEdqMCFN16YxKe6",
                "t3ZuaJziLM8xZ32rjDUzVjVtyYdDSz8GLWB",
                "t3bAtYWa4bi8VrtvqySxnbr5uqcG9czQGTZ",
                "t3dktADfb5Rmxncpe1HS5BRS5Gcj7MZWYBi",
                "t3hgskquvKKoCtvxw86yN7q8bzwRxNgUZmc",
                "t3R1VrLzwcxAZzkX4mX3KGbWpNsgtYtMntj",
                "t3ff6fhemqPMVujD3AQurxRxTdvS1pPSaa2",
                "t3cEUQFG3KYnFG6qYhPxSNgGi3HDjUPwC3J",
                "t3WR9F5U4QvUFqqx9zFmwT6xFqduqRRXnaa",
                "t3PYc1LWngrdUrJJbHkYPCKvJuvJjcm85Ch",
                "t3bgkjiUeatWNkhxY3cWyLbTxKksAfk561R",
                "t3Z5rrR8zahxUpZ8itmCKhMSfxiKjUp5Dk5",
                "t3PU1j7YW3fJ67jUbkGhSRto8qK2qXCUiW3",
                "t3S3yaT7EwNLaFZCamfsxxKwamQW2aRGEkh",
                "t3eutXKJ9tEaPSxZpmowhzKhPfJvmtwTEZK",
                "t3gbTb7brxLdVVghSPSd3ycGxzHbUpukeDm",
                "t3UCKW2LrHFqPMQFEbZn6FpjqnhAAbfpMYR",
                "t3NyHsrnYbqaySoQqEQRyTWkjvM2PLkU7Uu",
                "t3QEFL6acxuZwiXtW3YvV6njDVGjJ1qeaRo",
                "t3PdBRr2S1XTDzrV8bnZkXF3SJcrzHWe1wj",
                "t3ZWyRPpWRo23pKxTLtWsnfEKeq9T4XPxKM",
                "t3he6QytKCTydhpztykFsSsb9PmBT5JBZLi",
                "t3VWxWDsLb2TURNEP6tA1ZSeQzUmPKFNxRY",
                "t3NmWLvZkbciNAipauzsFRMxoZGqmtJksbz",
                "t3cKr4YxVPvPBG1mCvzaoTTdBNokohsRJ8n",
                "t3T3smGZn6BoSFXWWXa1RaoQdcyaFjMfuYK",
                "t3gkDUe9Gm4GGpjMk86TiJZqhztBVMiUSSA",
                "t3eretuBeBXFHe5jAqeSpUS1cpxVh51fAeb",
                "t3dN8g9zi2UGJdixGe9txeSxeofLS9t3yFQ",
                "t3S799pq9sYBFwccRecoTJ3SvQXRHPrHqvx",
                "t3fhYnv1S5dXwau7GED3c1XErzt4n4vDxmf",
                "t3cmE3vsBc5xfDJKXXZdpydCPSdZqt6AcNi",
                "t3h5fPdjJVHaH4HwynYDM5BB3J7uQaoUwKi",
                "t3Ma35c68BgRX8sdLDJ6WR1PCrKiWHG4Da9",
                "t3LokMKPL1J8rkJZvVpfuH7dLu6oUWqZKQK",
                "t3WFFGbEbhJWnASZxVLw2iTJBZfJGGX73mM",
                "t3L8GLEsUn4QHNaRYcX3EGyXmQ8kjpT1zTa",
                "t3PgfByBhaBSkH8uq4nYJ9ZBX4NhGCJBVYm",
                "t3WecsqKDhWXD4JAgBVcnaCC2itzyNZhJrv",
                "t3ZG9cSfopnsMQupKW5v9sTotjcP5P6RTbn",
                "t3hC1Ywb5zDwUYYV8LwhvF5rZ6m49jxXSG5",
                "t3VgMqDL15ZcyQDeqBsBW3W6rzfftrWP2yB",
                "t3LC94Y6BwLoDtBoK2NuewaEbnko1zvR9rm",
                "t3cWCUZJR3GtALaTcatrrpNJ3MGbMFVLRwQ",
                "t3YYF4rPLVxDcF9hHFsXyc5Yq1TFfbojCY6",
                "t3XHAGxRP2FNfhAjxGjxbrQPYtQQjc3RCQD",
            };

            // ZF and MG each use a single address repeated 48 times,
            // once for each funding period.
            std::vector<std::string> zf_addresses(48, "t3dvVE3SQEi7kqNzwrfNePxZ1d4hUyztBA1");
            std::vector<std::string> mg_addresses(48, "t3XyYW8yBFRuMnfvm5KLGFbEVz25kckZXym");

            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_BP,
                canopyActivation,
                nu6Activation,
                bp_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_ZF,
                canopyActivation,
                nu6Activation,
                zf_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_MG,
                canopyActivation,
                nu6Activation,
                mg_addresses);

            // ZIP 214 Revision 1
            // FPF uses a single address repeated 12 times, once for each funding period.
            std::vector<std::string> fpf_addresses(12, "t3cFfPt1Bcvgez9ZbMBFWeZsskxTkPzGCow");

            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG,
                nu6Activation,
                nu6_1Activation,
                fpf_addresses);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_DEFERRED,
                nu6Activation,
                nu6_1Activation);

            // ZIP 214 Revision 2
            // FPF uses a single address repeated 36 times, once for each funding period.
            std::vector<std::string> fpf_addresses_h3(36, "t3cFfPt1Bcvgez9ZbMBFWeZsskxTkPzGCow");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG_H3,
                nu6_1Activation,
                4406400,
                fpf_addresses_h3);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_CCF_H3,
                nu6_1Activation,
                4406400);

            // ZIP 271
            // For convenience of distribution, we split the lockbox contents into 10 equal chunks.
            std::string nu6_1_kho_address = "t3ev37Q2uL1sfTsiJQJiWJoFzQpDhmnUwYo";
            static const CAmount nu6_1_disbursement_amount = 78750 * COIN;
            static const CAmount nu6_1_chunk_amount = 7875 * COIN;
            static constexpr auto nu6_1_chunks = {
                Consensus::LD_ZIP271_NU6_1_CHUNK_1,
                Consensus::LD_ZIP271_NU6_1_CHUNK_2,
                Consensus::LD_ZIP271_NU6_1_CHUNK_3,
                Consensus::LD_ZIP271_NU6_1_CHUNK_4,
                Consensus::LD_ZIP271_NU6_1_CHUNK_5,
                Consensus::LD_ZIP271_NU6_1_CHUNK_6,
                Consensus::LD_ZIP271_NU6_1_CHUNK_7,
                Consensus::LD_ZIP271_NU6_1_CHUNK_8,
                Consensus::LD_ZIP271_NU6_1_CHUNK_9,
                Consensus::LD_ZIP271_NU6_1_CHUNK_10,
            };
            static_assert(nu6_1_chunk_amount * nu6_1_chunks.size() == nu6_1_disbursement_amount);
            for (auto idx : nu6_1_chunks) {
                consensus.AddZIP271LockboxDisbursement(
                    keyConstants,
                    idx,
                    Consensus::UPGRADE_NU6_1,
                    nu6_1_chunk_amount,
                    nu6_1_kho_address);
            }
#endif
        }

        // The best chain should have at least this much work.
        // From block 3308324, 2026-04-15.
           consensus.nMinimumChainWork = uint256S("0x00");

        /**
         * The message start string should be awesome! ⓩ❤
         */
        pchMessageStart[0] = 0x88;
        pchMessageStart[1] = 0xa7;
        pchMessageStart[2] = 0xa6;
        pchMessageStart[3] = 0xdd;
        nDefaultPort = 28233;
        nPruneAfterHeight = 100000;

        genesis = CreateGenesisBlock(
            1477641360,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000344"),
            ParseHex("0049188a69117502bf904542e7ec1416d4c23d39f63159468e475482899300d5432f3c709d77a31fc6c7052e1d1e38eaec9f6318f6e1dfe52622e37ffe67cc0dc64ce0f05b0a778c10322176a0fde4b74918f37500e21e2036a068154c37030bf957e1b578caf874a9068cfb646146a248edcad36b07ad69a1d372116ac72a41067d22286131cf2e930b09a2506ad53cdb7c333bab86608cf51eabd37fb8cc6468871e52bb15d8c901a1e33de96f2f3fe8cef439ce348c6a0989da27e209927244e307cb4e9bf9c57ad0d12259736753702b034cc3ddbcf15959a130484aec7cde26407d53affe136a6ab2fbcf336748ffe27904c37a00c51858ace30e1a94a12205b0b8522014232ea69cfde80f168ea03056aee6294e5a4f0ef6c37bdb50cc7e6cafb46dcc18580fc32419cbc976da0bb8bfe1eb6317d77fb64a6e862397e3a0b12fed81584907f2977f2c7bddf78500c9522fe7ccf0f964ba7305c8c450fd6e6c8bab03039e88c1d7b5471dda9fb2a6d2d78b4cf5b571049919abc54f1d8abaf6833ae362bdb093aa2ea5fe3dc63b8d7f8a0acee873fb20f70b86f249dae4d5fd93d305d1377542d3d96faf42c56b1b6994f2b1761bab991300e5927a8585d09c15b35a3f6d5ea901ad4aa6db080e71073f4f3badc038433386a0bbdf1613798ef61a0fd4feeee20a13228d44358c3dd6563ce95ff78310cbd2392c60d041ee1885bc9b557059bb4cba19b9136d172ce4453cd0b704046994d351f1cacf33f90519ff58da3b46efd1e486d89172c6fdc23b0cf595312954b5eb229cdf01ad30b96a7cf9281e7b38f91aa112ea7481ca9633494444e3e3a4ef573606a0bd26831e4b993cfa935c13d750c2c91f5d8ee2c0b399688120c3baab0918a53b9235151ebbc92a1d5c35adc8862683b66ad1dc95ad679a54b1c3b6105177503d4a8400c99b3709d9a37177b38324a8e398d9f6c5d4da6119ec9536df4c00075d9501db7d1826cb66279da600035b36673912e40f87f62372d6cac6f94b41111c8704c11190fef0a30fbd8131f510c9d3c0a9496f39420d15b0a2c4d88cd150d4a5b715fcffbf9a03de27f810f3b34557d607ab43938110bc780953a84f9fc9e00da5228512863e7e45fd91f52bbff348ba3d47f8e954e52d49389a808903d5269c4be3f585daf035525804c3f9775a68ef1d73a2c1f428e8c88ce23239606a16aea1884f193913113f527895c4180d02376bfc4033e0f2723e61326f1fd224204c7852a5371b8e69dd576d32f4d3db4b1b6c8cebabe768cd83346afc3ea511c2e47f1a1d831d73d0881954f985a74e9abdd2652c891b3873e7e5a9de6ae3c0453bdd39410abfe1fa1c3429e931dc3da54b3b622740ff1ea0f1bb18b51655dc563c723382a1b0c755a52270b277030add4bb3011fb981d85f9329c25ee2f92c77cf6a8dc559eb251c068c15694b6fdcb8a2a3916ee983d53c19358ab3259d73a8144cba77888993a1eb6a745d0ee1d57f0c3d4aa61b63d5e83108f7055d62c1fc5a717dbbe289068ee132f4dfa7652ecf835451f45feb08149d50122cd052c588d57719f4ddb599c6397042e30c196c650b815108b26763c1dfd707a1ab6f46a207783cd4471242aa51648a52d0c46054ff046c010db5715e8ed806ffb6c041371551f7f4f372e8b2c0fd0875b5808a09b5f6e66d533597ba7e8391cd3a59a2518abd38ad0cfaa95e4f859c40ac9f61839f2aefecc47d3c86760e02ba099ef5e86ff20f14941e6dc652e8043760a909f9e42ac76f8709c465c919a9f36d733b0d7d44180c50a35dfaddf8efed47681e63de9b0724ce5a2f550cbdae7f16c98498d1cd2496217c3951bb6b1c655b194c236846873bb0a0d5a5bb4a68a61df99450d147"),
            0x1f07ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0003c4a97174e882019f73889c3d61971aa34dfd4a6c5116efe4de665f3761e8"));
        assert(genesis.hashMerkleRoot == uint256S("0x7309317f6703debc23b59c785490623b651fe330046a4fd6f7fad07cf758be6c"));

        vFixedSeeds.clear();
        vSeeds.clear();


        fMiningRequiresPeers = true;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = false;

                checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock),
            (int64_t)genesis.nTime,  // * UNIX timestamp of last checkpoint block (genesis)
            0,                        // * total number of transactions between genesis and last checkpoint
            0                         // * estimated number of transactions per day after checkpoint
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 0;
        nSproutValuePoolCheckpointBalance = 0;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock.SetNull();

        // Chain supply checkpoint at NU6.1 activation (height 3146400).
        // This allows nodes with legacy block index data (written by zcashd
        // versions older than 5.4.0, which did not serialize nChainSupplyDelta)
        // to bootstrap nChainTotalSupply and nChainTransparentValue without
        // requiring a reindex. The other pool balances are included so that
        // we do not need to trust that the computed values from before the
        // checkpoint are correct.
        nChainSupplyCheckpointHeight = 0;
        nChainSupplyCheckpointTotalSupply = 0;
        nChainSupplyCheckpointTransparentValue = 0;
        nChainSupplyCheckpointSproutValue = 0;
        nChainSupplyCheckpointSaplingValue = 0;
        nChainSupplyCheckpointOrchardValue = 0;
        nChainSupplyCheckpointLockboxValue = 0;
        hashChainSupplyCheckpointBlock.SetNull();
        // ReweCoin: no founders reward on this chain. vFoundersRewardAddress
        // intentionally empty — safe because miner.cpp no longer calls
        // GetFoundersRewardScriptAtHeight/GetFoundersRewardAddressAtHeight.
        vFoundersRewardAddress = {};

        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));
    }
};
static CMainParams mainParams;

/**
 * Testnet (v3)
 */
class CTestNetParams : public CChainParams {
public:
    CTestNetParams() {
        keyConstants.strNetworkID = "test";
        strCurrencyUnits = "TAZ";
        keyConstants.bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = true;
        consensus.nSubsidySlowStartInterval = 20000;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 51;
        consensus.nMajorityRejectBlockOutdated = 75;
        consensus.nMajorityWindow = 400;
        const size_t N = 200, K = 9;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("07ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 32; // 32% adjustment down
        consensus.nPowMaxAdjustUp = 16; // 16% adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 299187;
        consensus.fPowNoRetargeting = false;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170007;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170050;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].hashActivationBlock = std::nullopt;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170110;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight = 1;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170130;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight = 3536500;
        consensus.nTemporaryOrchardDisablingSoftForkHeight = 4048500;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nProtocolVersion = 170150;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nActivationHeight = 4052000;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;

        // guarantees the first 2 characters, when base58 encoded, are "tm"
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x0E,0xA4};
        // guarantees the first 2 characters, when base58 encoded, are "t2"
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x0E,0xA9};
        // the first character, when base58 encoded, is "9" or "c" (as in Bitcoin)
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        // guarantees the first 2 characters, when base58 encoded, are "zt"
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        // guarantees the first 4 characters, when base58 encoded, are "ZiVt"
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        // guarantees the first 2 characters, when base58 encoded, are "ST"
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "rwtestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "rwviewtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "rwivktestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-rewecoin-test";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "rwxviewtestsapling";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "textest";

        // Testnet funding streams
        {
#if 0
            auto canopyActivation = consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight;
            auto nu6Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight;
            auto nu6_1Activation = consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight;

            // ZIP 214 Revision 0
            std::vector<std::string> bp_addresses = {
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t26ovBdKAJLtrvBsE2QGF4nqBkEuptuPFZz",
                "t2NNHrgPpE388atmWSF4DxAb3xAoW5Yp45M",
                "t2VMN28itPyMeMHBEd9Z1hm6YLkQcGA1Wwe",
                "t2CHa1TtdfUV8UYhNm7oxbzRyfr8616BYh2",
                "t2F77xtr28U96Z2bC53ZEdTnQSUAyDuoa67",
                "t2ARrzhbgcpoVBDPivUuj6PzXzDkTBPqfcT",
                "t278aQ8XbvFR15mecRguiJDQQVRNnkU8kJw",
                "t2Dp1BGnZsrTXZoEWLyjHmg3EPvmwBnPDGB",
                "t2KzeqXgf4ju33hiSqCuKDb8iHjPCjMq9iL",
                "t2Nyxqv1BiWY1eUSiuxVw36oveawYuo18tr",
                "t2DKFk5JRsVoiuinK8Ti6eM4Yp7v8BbfTyH",
                "t2CUaBca4k1x36SC4q8Nc8eBoqkMpF3CaLg",
                "t296SiKL7L5wvFmEdMxVLz1oYgd6fTfcbZj",
                "t29fBCFbhgsjL3XYEZ1yk1TUh7eTusB6dPg",
                "t2FGofLJXa419A76Gpf5ncxQB4gQXiQMXjK",
                "t2ExfrnRVnRiXDvxerQ8nZbcUQvNvAJA6Qu",
                "t28JUffLp47eKPRHKvwSPzX27i9ow8LSXHx",
                "t2JXWPtrtyL861rFWMZVtm3yfgxAf4H7uPA",
                "t2QdgbJoWfYHgyvEDEZBjHmgkr9yNJff3Hi",
                "t2QW43nkco8r32ZGRN6iw6eSzyDjkMwCV3n",
                "t2DgYDXMJTYLwNcxighQ9RCgPxMVATRcUdC",
                "t2Bop7dg33HGZx3wunnQzi2R2ntfpjuti3M",
                "t2HVeEwovcLq9RstAbYkqngXNEsCe2vjJh9",
                "t2HxbP5keQSx7p592zWQ5bJ5GrMmGDsV2Xa",
                "t2TJzUg2matao3mztBRJoWnJY6ekUau6tPD",
                "t29pMzxmo6wod25YhswcjKv3AFRNiBZHuhj",
                "t2QBQMRiJKYjshJpE6RhbF7GLo51yE6d4wZ",
                "t2F5RqnqguzZeiLtYHFx4yYfy6pDnut7tw5",
                "t2CHvyZANE7XCtg8AhZnrcHCC7Ys1jJhK13",
                "t2BRzpMdrGWZJ2upsaNQv6fSbkbTy7EitLo",
                "t2BFixHGQMAWDY67LyTN514xRAB94iEjXp3",
                "t2Uvz1iVPzBEWfQBH1p7NZJsFhD74tKaG8V",
                "t2CmFDj5q6rJSRZeHf1SdrowinyMNcj438n",
                "t2ErNvWEReTfPDBaNizjMPVssz66aVZh1hZ",
                "t2GeJQ8wBUiHKDVzVM5ZtKfY5reCg7CnASs",
                "t2L2eFtkKv1G6j55kLytKXTGuir4raAy3yr",
                "t2EK2b87dpPazb7VvmEGc8iR6SJ289RywGL",
                "t2DJ7RKeZJxdA4nZn8hRGXE8NUyTzjujph9",
                "t2K1pXo4eByuWpKLkssyMLe8QKUbxnfFC3H",
                "t2TB4mbSpuAcCWkH94Leb27FnRxo16AEHDg",
                "t2Phx4gVL4YRnNsH3jM1M7jE4Fo329E66Na",
                "t2VQZGmeNomN8c3USefeLL9nmU6M8x8CVzC",
                "t2RicCvTVTY5y9JkreSRv3Xs8q2K67YxHLi",
                "t2JrSLxTGc8wtPDe9hwbaeUjCrCfc4iZnDD",
                "t2Uh9Au1PDDSw117sAbGivKREkmMxVC5tZo",
                "t2FDwoJKLeEBMTy3oP7RLQ1Fihhvz49a3Bv",
                "t2FY18mrgtb7QLeHA8ShnxLXuW8cNQ2n1v8",
                "t2L15TkDYum7dnQRBqfvWdRe8Yw3jVy9z7g",
            };

            // ZF and MG use the same address for each funding period
            std::vector<std::string> zf_addresses(51, "t27eWDgjFYJGVXmzrXeVjnb5J3uXDM9xH9v");
            std::vector<std::string> mg_addresses(51, "t2Gvxv2uNM7hbbACjNox4H6DjByoKZ2Fa3P");

            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_BP,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                bp_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_ZF,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                zf_addresses);
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_ZIP214_MG,
                canopyActivation,
                2796000, // *not* the NU6 activation height
                mg_addresses);

            // ZIP 214 Revision 1
            // FPF uses a single address repeated 13 times, once for each funding period.
            // There are 13 periods because the start height does not align with a period boundary.
            std::vector<std::string> fpf_addresses(13, "t2HifwjUj9uyxr9bknR8LFuQbc98c3vkXtu");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG,
                nu6Activation,
                3396000,
                fpf_addresses);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_DEFERRED,
                nu6Activation,
                3396000);

            // ZIP 214 Revision 2
            // FPF uses a single address repeated 27 times, once for each funding period.
            // There are 27 periods because the start height is after the second halving
            // on testnet and does not align with a period boundary.
            std::vector<std::string> fpf_addresses_h3(27, "t2HifwjUj9uyxr9bknR8LFuQbc98c3vkXtu");
            consensus.AddZIP207FundingStream(
                keyConstants,
                Consensus::FS_FPF_ZCG_H3,
                nu6_1Activation,
                4476000,
                fpf_addresses_h3);
            consensus.AddZIP207LockboxStream(
                keyConstants,
                Consensus::FS_CCF_H3,
                nu6_1Activation,
                4476000);

            // ZIP 271
            // For testing purposes, we split the lockbox contents into 10 equal chunks.
            std::string nu6_1_kho_address = "t2RnBRiqrN1nW4ecZs1Fj3WWjNdnSs4kiX8";
            static const CAmount nu6_1_disbursement_amount = 78750 * COIN;
            static const CAmount nu6_1_chunk_amount = 7875 * COIN;
            static constexpr auto nu6_1_chunks = {
                Consensus::LD_ZIP271_NU6_1_CHUNK_1,
                Consensus::LD_ZIP271_NU6_1_CHUNK_2,
                Consensus::LD_ZIP271_NU6_1_CHUNK_3,
                Consensus::LD_ZIP271_NU6_1_CHUNK_4,
                Consensus::LD_ZIP271_NU6_1_CHUNK_5,
                Consensus::LD_ZIP271_NU6_1_CHUNK_6,
                Consensus::LD_ZIP271_NU6_1_CHUNK_7,
                Consensus::LD_ZIP271_NU6_1_CHUNK_8,
                Consensus::LD_ZIP271_NU6_1_CHUNK_9,
                Consensus::LD_ZIP271_NU6_1_CHUNK_10,
            };
            static_assert(nu6_1_chunk_amount * nu6_1_chunks.size() == nu6_1_disbursement_amount);
            for (auto idx : nu6_1_chunks) {
                consensus.AddZIP271LockboxDisbursement(
                    keyConstants,
                    idx,
                    Consensus::UPGRADE_NU6_1,
                    nu6_1_chunk_amount,
                    nu6_1_kho_address);
            }
#endif
        }

        // On testnet we activate this rule 6 blocks after Blossom activation. From block 299188 and
        // prior to Blossom activation, the testnet minimum-difficulty threshold was 15 minutes (i.e.
        // a minimum difficulty block can be mined if no block is mined normally within 15 minutes):
        // <https://zips.z.cash/zip-0205#change-to-difficulty-adjustment-on-testnet>
        // However the median-time-past is 6 blocks behind, and the worst-case time for 7 blocks at a
        // 15-minute spacing is ~105 minutes, which exceeds the limit imposed by the soft fork of
        // 90 minutes.
        //
        // After Blossom, the minimum difficulty threshold time is changed to 6 times the block target
        // spacing, which is 7.5 minutes:
        // <https://zips.z.cash/zip-0208#minimum-difficulty-blocks-on-the-test-network>
        // 7 times that is 52.5 minutes which is well within the limit imposed by the soft fork.

        static_assert(6 * Consensus::POST_BLOSSOM_POW_TARGET_SPACING * 7 < MAX_FUTURE_BLOCK_TIME_MTP - 60,
                      "MAX_FUTURE_BLOCK_TIME_MTP is too low given block target spacing");
        consensus.nFutureTimestampSoftForkHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight + 6;

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xa2;
        pchMessageStart[1] = 0x47;
        pchMessageStart[2] = 0x04;
        pchMessageStart[3] = 0x46;
        nDefaultPort = 28333;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1477648033,
            uint256S("0x000000000000000000000000000000000000000000000000000000000000001a"),
            ParseHex("000ad61313071dc98366608ec1643779f71dfd7a64029653f468868e4b96f0f2b5582ed59e5af4384b0d000c637be19fbda74243b07c2bdc2f3c4372233abe13ed9b14974ca5156ca3075ebde7203a203cdff74c0c1166c75978f433e7de65cb685de7e6159ffddfd2170be58966d2da47504fa3b47b9fd7a763451d4d65116b0695bd4af161e0c662d24d631a252c3ad867ef14134548f295afcd4a1c03646b721d1f5dcf1d75ef01afff6cbbd0fd8fd58fd1bad19b5249183e4e7e581d44df07d147bc8538b861dea821eeca34fc33c84d0fd06bf47a311601e73c37cad14c57ea5cf75ca73f106f221c118ba8af7501a1b0e13bc81d71a9bc71e81637c6a75b458f6841c781bcaed7133d62736fbf505054568c1b75cc61c3e845f5153263d193b95bba2322b933ba89287935de4aa2b4fbc721be2b8a14602d2b2d654a9a4ad767baf8f46f8f2a7789bb0e2de51e00f71f3e650d7d4ce95d110c305ebc3cd4a0f0ecd51624b4d6b2f98e79fc39f75df9e90b0dee92786ef5033b4c8c3009a6b258f6844cacacf31206f697524f088ac5dc0407bd326f5162679b990d1d4800ca433b18a8db1cb261b53ff2ed356685c19e016b5092d0b43027bd824c64fafbd36074d294c62187a413bf33301993f90c33a396c9ab3e7572b737e349acb2d52e901a9cbbbb79e3aad93439c2ed74bf0652e4517a7db50a1bc7a86d136ae0bb4d414574943571ae6b5b532428afea3022ca4d79d83813d8d97660a96787b6f63f0b79f7f8e51c7ea8e6c6330e63ba5f9f515edff2851cd65a6a346a0dc7511959928b6b84fa7265774cd70bc0b3f6e604cbb5d03ee2b4bc1f9326752d54cb0720848361e4d49137d11d13840947555d032a8c206216baf66e908a83bdca7e95ad2d3a3e5a0da45860ef1ad5cde64c66d909c11d0218a9cb33fcb1ba09c28a048c2b74c0d39bf3224653167fc42a115e6a3831e5221169fb680b35dec5e993bbfe72ff062c36ba186b10a1b3734e5c486183238721cf484c4ed836bcd1a2143a5e45959e7b99f4cda47160fec6c2081f9f80d115c2f37b1d9e86e5d65833dac169a4fe2fd8b21bb2270bdf42c8b0b41df339e3f6bfd403ce04a11b0e9b18ff36fd56891d35c35722409d965fd170ca5d8ec03836a79ab32bf039d24564e2d6b3bd6e13323189f106037a74c2a3b29d69ac62a16332f20566ca94d0c31af44cdab3e0383dc260b33bebf60f21af9b3834720e54b9dd365c1d6bc523614348253bde7883bd0d462e7d1ec9a98c226897f053056efd562da63d58a3980ab8435d43598cf3bbfff0e03c646a3506bbebd58e4052dca74ee8ade3c4e2b416d47849fd356c90753317ad7ed228d8d6d7b85d83e0f5c253dde6555bda923373943b0571d4359276545ee9db134946b0fec63804ac60b45e235d51248db2a9a364b9bddf05b4903914422cdfde49951afc4c82ba03fa39252ecc7b1ade05e36528d18b8e16f84bd1fd5ddc007257859f795d0ec17d2c1e53fb34f206f37668a4af9dbb3778b8fa0fa392827f861cb3de7f295f4df89d835d67fd341525074bf7670f57b948be22741cf9e198c8967cc1a41c394d57d58d68888d25763be7b90b1a88cc1fb0881e69d694671e16fb07285797fbee5f6a27529481930a0776a05b15387ade15e7b074fbc7d3921d9f6afb1613c41773ca3446f6774760c5d0d1e303f9bc3f172ec63662d45ab5d5811c7482563b5b362707fd646f3c745befb8cf6cc1a26b1784dcf6526f7cb83e40150fb36c4d57d2488503b775ce0b3a1fe5db39ff838515f547a6ae73dd55f93052a1ec33b3fcbf4c2c38ee31396e11817011d9a8a7b439892612ad7f9dbac38d6b1ca59492ad1093eeb149f2eaf532df5ec4bed0c1"),
            0x2007ffff, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0148188b2279922a528736172d8205efb4d23b3b351e72a4f352d25f393d34f1"));
        assert(genesis.hashMerkleRoot == uint256S("0x7309317f6703debc23b59c785490623b651fe330046a4fd6f7fad07cf758be6c"));

        vFixedSeeds.clear();
        vSeeds.clear();


        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = false;
        fRequireStandard = true;
        fMineBlocksOnDemand = false;
        fTestnetToBeDeprecatedFieldRPC = true;


        checkpointData = (CCheckpointData) {
            boost::assign::map_list_of
            (0, consensus.hashGenesisBlock)
            (38000, uint256S("0x001e9a2d2e2892b88e9998cf7b079b41d59dd085423a921fe8386cecc42287b8")),
            1486897419,  // * UNIX timestamp of last checkpoint block
            47163,       // * total number of transactions between genesis and last checkpoint
            715          //   total number of tx / (checkpoint block height / (24 * 24))
        };

        // Hardcoded fallback value for the Sprout shielded value pool balance
        // for nodes that have not reindexed since the introduction of monitoring
        // in #2795.
        nSproutValuePoolCheckpointHeight = 0;
        nSproutValuePoolCheckpointBalance = 0;
        fZIP209Enabled = true;
        hashSproutValuePoolCheckpointBlock = uint256S("0148188b2279922a528736172d8205efb4d23b3b351e72a4f352d25f393d34f1");

        // Chain supply checkpoint at NU6.1 activation (height 3536500).
        nChainSupplyCheckpointHeight = 3536500;
        nChainSupplyCheckpointTotalSupply = 1690647512835043;
        nChainSupplyCheckpointTransparentValue = 1499728640946163;
        nChainSupplyCheckpointSproutValue = 42832983037484;
        nChainSupplyCheckpointSaplingValue = 140562922195481;
        nChainSupplyCheckpointOrchardValue = 7522947905915;
        nChainSupplyCheckpointLockboxValue = 18750000;
        hashChainSupplyCheckpointBlock = uint256S("01b947c7556b23040dc6840e9d3e4c6d9478c67a87b9737a83be848729d6e0af");

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = {
            "t2UNzUUx8mWBCRYPRezvA363EYXyEpHokyi", "t2N9PH9Wk9xjqYg9iin1Ua3aekJqfAtE543", "t2NGQjYMQhFndDHguvUw4wZdNdsssA6K7x2", "t2ENg7hHVqqs9JwU5cgjvSbxnT2a9USNfhy",
            "t2BkYdVCHzvTJJUTx4yZB8qeegD8QsPx8bo", "t2J8q1xH1EuigJ52MfExyyjYtN3VgvshKDf", "t2Crq9mydTm37kZokC68HzT6yez3t2FBnFj", "t2EaMPUiQ1kthqcP5UEkF42CAFKJqXCkXC9",
            "t2F9dtQc63JDDyrhnfpzvVYTJcr57MkqA12", "t2LPirmnfYSZc481GgZBa6xUGcoovfytBnC", "t26xfxoSw2UV9Pe5o3C8V4YybQD4SESfxtp", "t2D3k4fNdErd66YxtvXEdft9xuLoKD7CcVo",
            "t2DWYBkxKNivdmsMiivNJzutaQGqmoRjRnL", "t2C3kFF9iQRxfc4B9zgbWo4dQLLqzqjpuGQ", "t2MnT5tzu9HSKcppRyUNwoTp8MUueuSGNaB", "t2AREsWdoW1F8EQYsScsjkgqobmgrkKeUkK",
            "t2Vf4wKcJ3ZFtLj4jezUUKkwYR92BLHn5UT", "t2K3fdViH6R5tRuXLphKyoYXyZhyWGghDNY", "t2VEn3KiKyHSGyzd3nDw6ESWtaCQHwuv9WC", "t2F8XouqdNMq6zzEvxQXHV1TjwZRHwRg8gC",
            "t2BS7Mrbaef3fA4xrmkvDisFVXVrRBnZ6Qj", "t2FuSwoLCdBVPwdZuYoHrEzxAb9qy4qjbnL", "t2SX3U8NtrT6gz5Db1AtQCSGjrpptr8JC6h", "t2V51gZNSoJ5kRL74bf9YTtbZuv8Fcqx2FH",
            "t2FyTsLjjdm4jeVwir4xzj7FAkUidbr1b4R", "t2EYbGLekmpqHyn8UBF6kqpahrYm7D6N1Le", "t2NQTrStZHtJECNFT3dUBLYA9AErxPCmkka", "t2GSWZZJzoesYxfPTWXkFn5UaxjiYxGBU2a",
            "t2RpffkzyLRevGM3w9aWdqMX6bd8uuAK3vn", "t2JzjoQqnuXtTGSN7k7yk5keURBGvYofh1d", "t2AEefc72ieTnsXKmgK2bZNckiwvZe3oPNL", "t2NNs3ZGZFsNj2wvmVd8BSwSfvETgiLrD8J",
            "t2ECCQPVcxUCSSQopdNquguEPE14HsVfcUn", "t2JabDUkG8TaqVKYfqDJ3rqkVdHKp6hwXvG", "t2FGzW5Zdc8Cy98ZKmRygsVGi6oKcmYir9n", "t2DUD8a21FtEFn42oVLp5NGbogY13uyjy9t",
            "t2UjVSd3zheHPgAkuX8WQW2CiC9xHQ8EvWp", "t2TBUAhELyHUn8i6SXYsXz5Lmy7kDzA1uT5", "t2Tz3uCyhP6eizUWDc3bGH7XUC9GQsEyQNc", "t2NysJSZtLwMLWEJ6MH3BsxRh6h27mNcsSy",
            "t2KXJVVyyrjVxxSeazbY9ksGyft4qsXUNm9", "t2J9YYtH31cveiLZzjaE4AcuwVho6qjTNzp", "t2QgvW4sP9zaGpPMH1GRzy7cpydmuRfB4AZ", "t2NDTJP9MosKpyFPHJmfjc5pGCvAU58XGa4",
            "t29pHDBWq7qN4EjwSEHg8wEqYe9pkmVrtRP", "t2Ez9KM8VJLuArcxuEkNRAkhNvidKkzXcjJ", "t2D5y7J5fpXajLbGrMBQkFg2mFN8fo3n8cX", "t2UV2wr1PTaUiybpkV3FdSdGxUJeZdZztyt",
            };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));
    }
};
static CTestNetParams testNetParams;

/**
 * Regression test
 */
class CRegTestParams : public CChainParams {
public:
    CRegTestParams() {
        keyConstants.strNetworkID = "regtest";
        strCurrencyUnits = "REG";
        keyConstants.bip44CoinType = 1;
        consensus.fCoinbaseMustBeShielded = false;
        consensus.nSubsidySlowStartInterval = 0;
        consensus.nPreBlossomSubsidyHalvingInterval = Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL;
        consensus.nPostBlossomSubsidyHalvingInterval = POST_BLOSSOM_HALVING_INTERVAL(Consensus::PRE_BLOSSOM_REGTEST_HALVING_INTERVAL);
        consensus.nMajorityEnforceBlockUpgrade = 750;
        consensus.nMajorityRejectBlockOutdated = 950;
        consensus.nMajorityWindow = 1000;
        const size_t N = 48, K = 5;
        static_assert(equihash_parameters_acceptable(N, K));
        consensus.nEquihashN = N;
        consensus.nEquihashK = K;
        consensus.powLimit = uint256S("0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f0f"); // if this is any larger, the for loop in GetNextWorkRequired can overflow bnTot
        consensus.nPowAveragingWindow = 17;
        assert(maxUint/UintToArith256(consensus.powLimit) >= consensus.nPowAveragingWindow);
        consensus.nPowMaxAdjustDown = 0; // Turn off adjustment down
        consensus.nPowMaxAdjustUp = 0; // Turn off adjustment up
        consensus.nPreBlossomPowTargetSpacing = Consensus::PRE_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPostBlossomPowTargetSpacing = Consensus::POST_BLOSSOM_POW_TARGET_SPACING;
        consensus.nPowAllowMinDifficultyBlocksAfterHeight = 0;
        consensus.fPowNoRetargeting = true;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::BASE_SPROUT].nActivationHeight =
            Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nProtocolVersion = 170002;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nProtocolVersion = 170003;
        consensus.vUpgrades[Consensus::UPGRADE_OVERWINTER].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nProtocolVersion = 170006;
        consensus.vUpgrades[Consensus::UPGRADE_SAPLING].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nProtocolVersion = 170008;
        consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nProtocolVersion = 170010;
        consensus.vUpgrades[Consensus::UPGRADE_HEARTWOOD].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nProtocolVersion = 170012;
        consensus.vUpgrades[Consensus::UPGRADE_CANOPY].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nProtocolVersion = 170050;
        consensus.vUpgrades[Consensus::UPGRADE_NU5].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nProtocolVersion = 170110;
        consensus.vUpgrades[Consensus::UPGRADE_NU6].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nProtocolVersion = 170130;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_1].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.nTemporaryOrchardDisablingSoftForkHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nProtocolVersion = 170150;
        consensus.vUpgrades[Consensus::UPGRADE_NU6_2].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nProtocolVersion = 0x7FFFFFFF;
        consensus.vUpgrades[Consensus::UPGRADE_ZFUTURE].nActivationHeight =
            Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.nFundingPeriodLength = consensus.nPostBlossomSubsidyHalvingInterval / 48;
        // Defined funding streams can be enabled with node config flags.

        // These prefixes are the same as the testnet prefixes
        keyConstants.base58Prefixes[PUBKEY_ADDRESS]     = {0x0E,0xA4};
        keyConstants.base58Prefixes[SCRIPT_ADDRESS]     = {0x0E,0xA9};
        keyConstants.base58Prefixes[SECRET_KEY]         = {0xEF};
        // do not rely on these BIP32 prefixes; they are not specified and may change
        keyConstants.base58Prefixes[EXT_PUBLIC_KEY]     = {0x04,0x35,0x87,0xCF};
        keyConstants.base58Prefixes[EXT_SECRET_KEY]     = {0x04,0x35,0x83,0x94};
        keyConstants.base58Prefixes[ZCPAYMENT_ADDRESS]  = {0x16,0xB6};
        keyConstants.base58Prefixes[ZCVIEWING_KEY]      = {0xA8,0xAC,0x0C};
        keyConstants.base58Prefixes[ZCSPENDING_KEY]     = {0xAC,0x08};

        keyConstants.bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "rwregtestsapling";
        keyConstants.bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "rwviewregtestsapling";
        keyConstants.bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "rwivkregtestsapling";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "secret-extended-key-rewecoin-regtest";
        keyConstants.bech32HRPs[SAPLING_EXTENDED_FVK]         = "rwxviewregtestsapling";

        keyConstants.bech32mHRPs[TEX_ADDRESS]                 = "texregtest";

        // The best chain should have at least this much work.
        consensus.nMinimumChainWork = uint256S("0x00");

        pchMessageStart[0] = 0xd6;
        pchMessageStart[1] = 0xc8;
        pchMessageStart[2] = 0x2a;
        pchMessageStart[3] = 0x63;
        nDefaultPort = 28444;
        nPruneAfterHeight = 1000;

        genesis = CreateGenesisBlock(
            1296688602,
            uint256S("0x0000000000000000000000000000000000000000000000000000000000000006"),
            ParseHex("00b4cac920ba0a17e22073dcff564da6695c1f5c721ed6ae070fc8614134df16cd6e4528"),
            0x200f0f0f, 4, 0);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x00733a3f518252e8d714c80b4acf8f86bb8dfbdceef94b49c5e43dd9348654d6"));
        assert(genesis.hashMerkleRoot == uint256S("0x7309317f6703debc23b59c785490623b651fe330046a4fd6f7fad07cf758be6c"));

        vFixedSeeds.clear(); //!< Regtest mode doesn't have any fixed seeds.
        vSeeds.clear();      //!< Regtest mode doesn't have any DNS seeds.

        fMiningRequiresPeers = false;
        fDefaultConsistencyChecks = true;
        fRequireStandard = false;
        fMineBlocksOnDemand = true;
        fTestnetToBeDeprecatedFieldRPC = false;

        checkpointData = (CCheckpointData){
            boost::assign::map_list_of
            ( 0, uint256S("0f9188f13cb7b2c71f2a335e3a4fc328bf5beb436012afca590b1a11466e2206")),
            0,
            0,
            0
        };

        // Founders reward script expects a vector of 2-of-3 multisig addresses
        vFoundersRewardAddress = { "T3U1UVMsEmNzYnXeQo24a8G9acAv7rhhfdG" };
        assert(vFoundersRewardAddress.size() <= consensus.GetLastFoundersRewardBlockHeight(0));

        // do not require the wallet backup to be confirmed in regtest mode
        fRequireWalletBackup = false;
    }

    void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
    {
        assert(idx > Consensus::BASE_SPROUT && idx < Consensus::MAX_NETWORK_UPGRADES);
        consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
    }

    void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
    {
        assert(idx >= Consensus::FIRST_FUNDING_STREAM && idx < Consensus::MAX_FUNDING_STREAMS);
        consensus.vFundingStreams[idx] = fs;
    }

    void UpdateOnetimeLockboxDisbursementParameters(
        Consensus::OnetimeLockboxDisbursementIndex idx,
        Consensus::OnetimeLockboxDisbursement ld)
    {
        assert(idx >= Consensus::FIRST_ONETIME_LOCKBOX_DISBURSEMENT && idx < Consensus::MAX_ONETIME_LOCKBOX_DISBURSEMENTS);
        consensus.vOnetimeLockboxDisbursements[idx] = ld;
    }

    void UpdateRegtestPow(
        int64_t nPowMaxAdjustDown,
        int64_t nPowMaxAdjustUp,
        uint256 powLimit,
        bool noRetargeting)
    {
        consensus.nPowMaxAdjustDown = nPowMaxAdjustDown;
        consensus.nPowMaxAdjustUp = nPowMaxAdjustUp;
        consensus.powLimit = powLimit;
        consensus.fPowNoRetargeting = noRetargeting;
    }

    void UpdateTemporaryOrchardDisablingSoftForkHeight(int nHeight)
    {
        consensus.nTemporaryOrchardDisablingSoftForkHeight = nHeight;
    }

    void SetRegTestZIP209Enabled() {
        fZIP209Enabled = true;
    }

    void SetRegTestAllowLegacyChainSupplyData() {
        fRegTestAllowLegacyChainSupplyData = true;
    }

    void SetRegTestChainSupplyCheckpoint() {
        // Hardcoded checkpoint at height 200 (the tip of the regtest caches).
        // The block hash is left null; FallbackChainSupplyCheckpoint
        // skips the hash check when it is null.
        // Values from the sprout regtest cache at height 200.
        // The sprout cache has 4 × 50 ZEC = 200 ZEC shielded into Sprout.
        nChainSupplyCheckpointHeight = 200;
        nChainSupplyCheckpointTotalSupply = 214375000000;
        nChainSupplyCheckpointTransparentValue = 194375000000;
        nChainSupplyCheckpointSproutValue = 20000000000;
        nChainSupplyCheckpointSaplingValue = 0;
        nChainSupplyCheckpointOrchardValue = 0;
        nChainSupplyCheckpointLockboxValue = 0;
        hashChainSupplyCheckpointBlock.SetNull();
    }
};
static CRegTestParams regTestParams;

static const CChainParams* pCurrentParams = nullptr;

const CChainParams& Params() {
    assert(pCurrentParams);
    return *pCurrentParams;
}

const CChainParams& Params(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
            return mainParams;
    else if (chain == CBaseChainParams::TESTNET)
            return testNetParams;
    else if (chain == CBaseChainParams::REGTEST)
            return regTestParams;
    else
        throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    pCurrentParams = &Params(network);

    // Some python qa rpc tests need to enforce the coinbase consensus rule
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestshieldcoinbase")) {
        regTestParams.SetRegTestCoinbaseMustBeShielded();
    }

    // When a developer is debugging turnstile violations in regtest mode, enable ZIP209
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-developersetpoolsizezero")) {
        regTestParams.SetRegTestZIP209Enabled();
    }

    // Allow regtest tests that use legacy block index data (lacking nChainSupplyDelta)
    // to skip the supply consistency check rather than aborting the node.
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestallowlegacychainsupplydata")) {
        regTestParams.SetRegTestAllowLegacyChainSupplyData();
    }

    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestchainsupplycheckpoint")) {
        regTestParams.SetRegTestChainSupplyCheckpoint();
    }

    // Enable ZIP 209 enforcement without zeroing shielded pool balances.
    if (network == CBaseChainParams::REGTEST && mapArgs.count("-regtestenablezip209")) {
        regTestParams.SetRegTestZIP209Enabled();
    }
}


// Block height must be >0 and <=last founders reward block height
// Index variable i ranges from 0 - (vFoundersRewardAddress.size()-1)
std::string CChainParams::GetFoundersRewardAddressAtHeight(int nHeight) const {
    int preBlossomMaxHeight = consensus.GetLastFoundersRewardBlockHeight(0);
    // zip208
    // FounderAddressAdjustedHeight(height) :=
    // height, if not IsBlossomActivated(height)
    // BlossomActivationHeight + floor((height - BlossomActivationHeight) / BlossomPoWTargetSpacingRatio), otherwise
    bool blossomActive = consensus.NetworkUpgradeActive(nHeight, Consensus::UPGRADE_BLOSSOM);
    if (blossomActive) {
        int blossomActivationHeight = consensus.vUpgrades[Consensus::UPGRADE_BLOSSOM].nActivationHeight;
        nHeight = blossomActivationHeight + ((nHeight - blossomActivationHeight) / Consensus::BLOSSOM_POW_TARGET_SPACING_RATIO);
    }
    assert(nHeight > 0 && nHeight <= preBlossomMaxHeight);
    size_t addressChangeInterval = (preBlossomMaxHeight + vFoundersRewardAddress.size()) / vFoundersRewardAddress.size();
    size_t i = nHeight / addressChangeInterval;
    return vFoundersRewardAddress[i];
}

// Block height must be >0 and <=last founders reward block height
// The founders reward address is expected to be a multisig (P2SH) address
CScript CChainParams::GetFoundersRewardScriptAtHeight(int nHeight) const {
    assert(nHeight > 0 && nHeight <= consensus.GetLastFoundersRewardBlockHeight(nHeight));

    KeyIO keyIO(*this);
    auto address = keyIO.DecodePaymentAddress(GetFoundersRewardAddressAtHeight(nHeight).c_str());
    assert(address.has_value());
    assert(std::holds_alternative<CScriptID>(address.value()));
    CScriptID scriptID = std::get<CScriptID>(address.value());
    CScript script = CScript() << OP_HASH160 << ToByteVector(scriptID) << OP_EQUAL;
    return script;
}

std::string CChainParams::GetFoundersRewardAddressAtIndex(int i) const {
    assert(i >= 0 && i < vFoundersRewardAddress.size());
    return vFoundersRewardAddress[i];
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    regTestParams.UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}

void UpdateFundingStreamParameters(Consensus::FundingStreamIndex idx, Consensus::FundingStream fs)
{
    regTestParams.UpdateFundingStreamParameters(idx, fs);
}

void UpdateOnetimeLockboxDisbursementParameters(
    Consensus::OnetimeLockboxDisbursementIndex idx,
    Consensus::OnetimeLockboxDisbursement ld)
{
    regTestParams.UpdateOnetimeLockboxDisbursementParameters(idx, ld);
}

void UpdateRegtestPow(
    int64_t nPowMaxAdjustDown,
    int64_t nPowMaxAdjustUp,
    uint256 powLimit,
    bool noRetargeting)
{
    regTestParams.UpdateRegtestPow(nPowMaxAdjustDown, nPowMaxAdjustUp, powLimit, noRetargeting);
}

void UpdateRegtestTemporaryOrchardDisablingSoftForkHeight(int nHeight)
{
    regTestParams.UpdateTemporaryOrchardDisablingSoftForkHeight(nHeight);
}
