// Temporary tool: mines new genesis blocks for ReweCoin (mainnet/testnet/regtest).
#include <gtest/gtest.h>

#include "arith_uint256.h"
#include "chainparams.h"
#include "consensus/merkle.h"
#include "crypto/equihash.h"
#include "pow/tromp/equi.h"
#include "primitives/block.h"
#include "primitives/transaction.h"
#include "script/script.h"
#include "streams.h"
#include "uint256.h"
#include "util/strencodings.h"
#include "version.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <vector>

namespace {

CBlock BuildCandidate(const char* pszTimestamp, uint32_t nTime, uint32_t nBits, int32_t nVersion)
{
    CScript genesisOutputScript = CScript() << ParseHex("04678afdb0fe5548271967f1a67130b7105cd6a828e03909a67962e0ea1f61deb649f6bc3f4cef38c4f35504e51ec112de5c384df7ba0b8d578a4c702b6bf11d5f") << OP_CHECKSIG;

    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 520617983 << CScriptNum(4)
        << std::vector<unsigned char>((const unsigned char*)pszTimestamp,
                                       (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = 0;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.nTime = nTime;
    genesis.nBits = nBits;
    genesis.nVersion = nVersion;
    genesis.vtx.push_back(txNew);
    genesis.hashPrevBlock.SetNull();
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

void MineGenesis(const char* label, const char* pszTimestamp, uint32_t nTime, uint32_t nBits,
                  unsigned int n, unsigned int k)
{
    CBlock pblock = BuildCandidate(pszTimestamp, nTime, nBits, 4);

    arith_uint256 hashTarget = arith_uint256().SetCompact(nBits);
    const char* rewe_st = getenv("REWE_START");
    uint64_t rewe_tried = 0;
    arith_uint256 nonceArith((uint64_t)(rewe_st ? strtoull(rewe_st, NULL, 10) : 0));
    bool found = false;

    while (!found) {
        pblock.nNonce = ArithToUint256(nonceArith);

        CEquihashInput I{pblock};
        CDataStream ss(SER_NETWORK, PROTOCOL_VERSION);
        ss << I;

        eh_HashState state = EhInitialiseState(n, k);
        state.Update((unsigned char*)&ss[0], ss.size());

        eh_HashState curr_state = state;
        curr_state.Update(pblock.nNonce.begin(), pblock.nNonce.size());

        std::function<bool(std::vector<unsigned char>)> validBlock =
            [&](std::vector<unsigned char> soln) {
                pblock.nSolution = soln;
                if (UintToArith256(pblock.GetHash()) > hashTarget) {
                    return false;
                }
                found = true;
                return true;
            };

        if (n == 200 && k == 9) {
            std::function<void()> incrementRuns = [](){};
            std::function<bool(size_t, const std::vector<uint32_t>&)> checkSolution =
                [&](size_t s, const std::vector<uint32_t>& index_vector) {
                    return validBlock(GetMinimalFromIndices(index_vector, DIGITBITS));
                };
            equihash_solve(curr_state.inner, incrementRuns, checkSolution);
        } else {
            std::function<bool(EhSolverCancelCheck)> cancelled = [](EhSolverCancelCheck) { return false; };
            try {
                EhOptimisedSolve(n, k, curr_state, validBlock, cancelled);
            } catch (...) {
            }
        }

        if (!found) {
            nonceArith += 1;
            if ((++rewe_tried % 25) == 0) { fprintf(stderr, "tried %llu nonces\n", (unsigned long long)rewe_tried); }
        }
    }

    printf("\n==================== %s ====================\n", label);
    printf("nTime      = %u\n", pblock.nTime);
    printf("nBits      = 0x%08x\n", pblock.nBits);
    printf("nNonce     = uint256S(\"0x%s\")\n", pblock.nNonce.GetHex().c_str());
    printf("nSolution  = ParseHex(\"%s\")\n", HexStr(pblock.nSolution).c_str());
    printf("genesisHash = uint256S(\"0x%s\")\n", pblock.GetHash().GetHex().c_str());
    printf("merkleRoot  = uint256S(\"0x%s\")\n", pblock.hashMerkleRoot.GetHex().c_str());
    fflush(stdout);
}

} // namespace

TEST(mine_genesis, ReweCoin)
{
    const char* pszTimestamp = "ReweCoin - A new chain begins - Sep 2026";

    MineGenesis("MAINNET", pszTimestamp, (uint32_t)strtoul(getenv("REWE_NTIME"), NULL, 10), 0x1f07ffff, 200, 9);
    // testnet skipped
    // regtest skipped
}
