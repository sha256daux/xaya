// Copyright (c) 2015-2017 The Bitcoin Core developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <chain.h>
#include <chainparams.h>
#include <pow.h>
#include <powdata.h>
#include <random.h>
#include <util.h>
#include <test/test_bitcoin.h>

#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(pow_tests, BasicTestingSetup)

#if 0
FIXME: Add updated tests for DGW difficulty algo.

/* Test calculation of next difficulty target with no constraints applying */
BOOST_AUTO_TEST_CASE(get_next_work)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1261130161; // Block #30240
    CBlockIndex pindexLast;
    pindexLast.nHeight = 32255;
    pindexLast.nTime = 1262152739;  // Block #32255
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00d86aU);
}

/* Test the constraint on the upper bound for next work */
BOOST_AUTO_TEST_CASE(get_next_work_pow_limit)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1231006505; // Block #0
    CBlockIndex pindexLast;
    pindexLast.nHeight = 2015;
    pindexLast.nTime = 1233061996;  // Block #2015
    pindexLast.nBits = 0x1d00ffff;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00ffffU);
}

/* Test the constraint on the lower bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_lower_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1279008237; // Block #66528
    CBlockIndex pindexLast;
    pindexLast.nHeight = 68543;
    pindexLast.nTime = 1279297671;  // Block #68543
    pindexLast.nBits = 0x1c05a3f4;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1c0168fdU);
}

/* Test the constraint on the upper bound for actual time taken */
BOOST_AUTO_TEST_CASE(get_next_work_upper_limit_actual)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    int64_t nLastRetargetTime = 1263163443; // NOTE: Not an actual block time
    CBlockIndex pindexLast;
    pindexLast.nHeight = 46367;
    pindexLast.nTime = 1269211443;  // Block #46367
    pindexLast.nBits = 0x1c387f6f;
    BOOST_CHECK_EQUAL(CalculateNextWorkRequired(&pindexLast, nLastRetargetTime, chainParams->GetConsensus()), 0x1d00e1fdU);
}

#endif // Disabled tests for difficulty algo.

BOOST_AUTO_TEST_CASE(GetBlockProofEquivalentTime_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    std::vector<CBlockIndex> blocks(10000);
    for (int i = 0; i < 10000; i++) {
        blocks[i].pprev = i ? &blocks[i - 1] : nullptr;
        blocks[i].nHeight = i;
        /* During one nPowTargetSpacing, we get blocks *from each* algorithm.
           Because of that, the actual spacing between two blocks (no matter
           what the algo) is half of the target spacing.  */
        blocks[i].nTime = 1269211443 + i * chainParams->GetConsensus().nPowTargetSpacing / 2;
        blocks[i].nBits = 0x207fffff; /* target 0x7fffff000... */
        if (i % 2 == 0)
          blocks[i].algo = PowAlgo::SHA256D;
        else
          blocks[i].algo = PowAlgo::NEOSCRYPT;
        blocks[i].nChainWork = i ? blocks[i - 1].nChainWork + GetBlockProof(blocks[i - 1]) : arith_uint256(0);
    }

    for (int j = 0; j < 1000; j++) {
        CBlockIndex *p1 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p2 = &blocks[InsecureRandRange(10000)];
        CBlockIndex *p3 = &blocks[InsecureRandRange(10000)];

        const int64_t wdiff = GetBlockProofEquivalentTime(*p1, *p2, *p3, chainParams->GetConsensus());
        const int64_t tdiff = p1->GetBlockTime() - p2->GetBlockTime();
        /* Depending on which blocks exactly are chosen, there might be
           "off-by-one" type differences in the number of blocks of each algo.
           Thus we allow for a little mismatch.  */
        BOOST_CHECK(std::abs(wdiff - tdiff) < chainParams->GetConsensus().nPowTargetSpacing);
    }
}

BOOST_AUTO_TEST_SUITE_END()
