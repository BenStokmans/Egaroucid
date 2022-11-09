/*
    Egaroucid Project

    @file setting.hpp
        Main settings of Egaroucid
    @date 2021-2022
    @author Takuto Yamana (a.k.a. Nyanyan)
    @license GPL-3.0 license
*/

// use SIMD
#define USE_SIMD true

// vertical mirror
#define USE_FAST_VERTICAL_MIRROR true

// parity ordering
#define USE_END_PO true

// stability cut
#define USE_END_SC true

// prob cut
#define USE_MID_MPC true

// pop_count
#define USE_BUILTIN_POPCOUNT true

// NTZ
#define USE_BUILTIN_NTZ true
#define USE_MINUS_NTZ false

// last parity ordering optimization
#define LAST_PO_OPTIMIZE true

// use probcut to predict it seems to be an all node
#define USE_ALL_NODE_PREDICTION true

// use search algs
#define USE_NEGA_ALPHA_ORDERING false
#define USE_NEGA_ALPHA_END false
#define USE_NEGA_ALPHA_END_FAST false

// use parallel clog search
#define USE_PARALLEL_CLOG_SEARCH true

// use SIMD in evaluation (pattern) function
#define USE_SIMD_EVALUATION true
#define USE_SIMD_DEBUG false

// move ordering
#define USE_AUTO_OPTIMIZED_MOVE_ORDERING_MID true
#define USE_AUTO_OPTIMIZED_MOVE_ORDERING_NWS true

// use bit gather optimization
#define USE_BIT_GATHER_OPTIMIZE true

// evaluation harness
#define USE_EVALUATION_HARNESS false