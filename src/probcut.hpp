/*
    Egaroucid Project

    @date 2021-2022
    @author Takuto Yamana (a.k.a Nyanyan)
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "util.hpp"

using namespace std;

#define PROBCUT_SHALLOW_IGNORE 5

#define ALL_NODE_CHECK_MPCT 1.8

#define probcut_a -0.00213107112213114
#define probcut_b -0.016798379790035622
#define probcut_c 0.01679837977996806
#define probcut_d -12.453803859134524
#define probcut_e -28.662952084310923
#define probcut_f -27.001850455790116
#define probcut_g 4.10046008079935

#define probcut_end_a 0.20405456811754308
#define probcut_end_b 0.19959570867591253
//#define probcut_end_c 6.238001249226975e-05
#define probcut_end_d -0.001252230777163415
#define probcut_end_e -0.47361044068293007
#define probcut_end_f 11.197678868342406

inline double probcut_sigma(int n_stones, int depth1, int depth2){
    double w = n_stones;
    double x = depth1;
    double y = depth1 - depth2;
    double res = probcut_a * w + probcut_b * x + probcut_c * y;
    res = probcut_d * res * res * res + probcut_e * res * res + probcut_f * res + probcut_g;
    return res;
}

inline double probcut_sigma_depth0(int n_stones, int depth1){
    double w = n_stones;
    double x = depth1;
    double res = probcut_a * w + probcut_b * x + probcut_c * x;
    res = probcut_d * res * res * res + probcut_e * res * res + probcut_f * res + probcut_g;
    return res;
}

inline double probcut_sigma_end(int n_stones, int depth){
    double x = n_stones;
    double y = depth;
    double res = probcut_end_a * x + probcut_end_b * y;
    //res = probcut_end_c * res * res * res + probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    res = probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    return res;
}

inline double probcut_sigma_end_depth0(int n_stones){
    double x = n_stones;
    double res = probcut_end_a * x;
    //res = probcut_end_c * res * res * res + probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    res = probcut_end_d * res * res + probcut_end_e * res + probcut_end_f;
    return res;
}

inline int nega_alpha_eval1_nws(Search *search, int alpha, bool skipped, const bool *searching);
#if MID_FAST_DEPTH > 1
    int nega_alpha_nws(Search *search, int alpha, int depth, bool skipped, const bool *searching);
#endif
#if USE_NEGA_ALPHA_ORDERING
    int nega_alpha_ordering(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching);
#endif
int nega_alpha_ordering_nws(Search *search, int alpha, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching);

inline bool mpc(Search *search, int alpha, int beta, int depth, uint64_t legal, bool is_end_search, int *v, const bool *searching){
    if (search->first_depth - depth < PROBCUT_SHALLOW_IGNORE)
        return false;
    int search_depth;
    if (is_end_search)
        search_depth = ((depth >> 4) & 0xFE) ^ (depth & 1);
    else
        search_depth = ((depth >> 2) & 0xFE) ^ (depth & 1);
    int error_depth0, error_search;
    if (is_end_search){
        //alpha -= alpha & 1;
        //beta += beta & 1;
        error_depth0 = search->mpct * probcut_sigma_end_depth0(search->n_discs) + 0.999;
        error_search = search->mpct * probcut_sigma_end(search->n_discs, search_depth) + 0.999;
    } else{
        error_depth0 = search->mpct * probcut_sigma_depth0(search->n_discs, depth) + 0.999;
        error_search = search->mpct * probcut_sigma(search->n_discs, depth, search_depth) + 0.999;
    }
    if (alpha - error_depth0 < -SCORE_MAX && SCORE_MAX < beta + error_depth0)
        return false;
    const int alpha_mpc = alpha - error_search;
    const int beta_mpc = beta + error_search;
    const int depth0_value = mid_evaluate_diff(search);
    bool res;
    if (depth0_value >= beta + error_depth0 && beta_mpc <= SCORE_MAX){
        switch(search_depth){
            case 0:
                res = true;
                break;
            case 1:
                res = nega_alpha_eval1_nws(search, beta_mpc - 1, false, searching) >= beta_mpc;
                break;
            default:
                #if MID_FAST_DEPTH > 1
                    if (search_depth <= MID_FAST_DEPTH)
                        res = nega_alpha_nws(search, beta_mpc - 1, search_depth, false, searching) >= beta_mpc;
                    else{
                        search->use_mpc = false;
                            res = nega_alpha_ordering_nws(search, beta_mpc - 1, search_depth, false, legal, false, searching) >= beta_mpc;
                        search->use_mpc = true;
                    }
                #else
                    search->use_mpc = false;
                        res = nega_alpha_ordering_nws(search, beta_mpc - 1, search_depth, false, legal, false, searching) >= beta_mpc;
                    search->use_mpc = true;
                #endif
                break;
        }
        if (res){
            *v = beta;
            return true;
        }
    } else if (depth0_value <= alpha - error_depth0 && alpha_mpc >= -SCORE_MAX){
        switch(search_depth){
            case 0:
                res = true;
                break;
            case 1:
                res = nega_alpha_eval1_nws(search, alpha_mpc, false, searching) <= alpha_mpc;
                break;
            default:
                #if MID_FAST_DEPTH > 1
                    if (search_depth <= MID_FAST_DEPTH)
                        res = nega_alpha_nws(search, alpha_mpc, search_depth, false, searching) <= alpha_mpc;
                    else{
                        search->use_mpc = false;
                            res = nega_alpha_ordering_nws(search, alpha_mpc, search_depth, false, legal, false, searching) <= alpha_mpc;
                        search->use_mpc = true;
                    }
                #else
                    search->use_mpc = false;
                        res = nega_alpha_ordering_nws(search, alpha_mpc, search_depth, false, legal, false, searching) <= alpha_mpc;
                    search->use_mpc = true;
                #endif
                break;
        }
        if (res){
            *v = alpha;
            return true;
        }
    }
    return false;
}

#if USE_ALL_NODE_PREDICTION
    inline bool is_like_all_node(Search *search, int alpha, int depth, uint64_t legal, bool is_end_search, const bool *searching){
        if (search->first_depth - depth < PROBCUT_SHALLOW_IGNORE)
            return false;
        bool res = false;
        int search_depth;
        if (is_end_search)
            search_depth = ((depth >> 4) & 0xFE) ^ (depth & 1);
        else
            search_depth = ((depth >> 2) & 0xFE) ^ (depth & 1);
        const int depth0_value = mid_evaluate_diff(search);
        int error_depth0, error_search;
        double mpct = min(ALL_NODE_CHECK_MPCT, search->mpct);
        if (is_end_search){
            alpha -= alpha & 1;
            error_depth0 = ceil(mpct * probcut_sigma_end_depth0(search->n_discs));
            error_search = ceil(mpct * probcut_sigma_end(search->n_discs, search_depth));
        } else{
            error_depth0 = ceil(mpct * probcut_sigma_depth0(search->n_discs, depth));
            error_search = ceil(mpct * probcut_sigma(search->n_discs, depth, search_depth));
        }
        if (depth0_value <= alpha - error_depth0 && alpha - error_search >= -SCORE_MAX){
            switch(search_depth){
                case 0:
                    res = true;
                    break;
                case 1:
                    res = nega_alpha_eval1_nws(search, alpha - error_search, false, searching) <= alpha - error_search;
                    break;
                default:
                    #if MID_FAST_DEPTH > 1
                        if (search_depth <= MID_FAST_DEPTH)
                            res = nega_alpha_nws(search, alpha - error_search, search_depth, false, searching) <= alpha - error_search;
                        else{
                            search->use_mpc = false;
                                res = nega_alpha_ordering_nws(search, alpha - error_search, search_depth, false, legal, false, searching) <= alpha - error_search;
                            search->use_mpc = true;
                        }
                    #else
                        search->use_mpc = false;
                            res = nega_alpha_ordering_nws(search, alpha - error_search, search_depth, false, legal, false, searching) <= alpha - error_search;
                        search->use_mpc = true;
                    #endif
                    break;
            }
            return res;
        }
        return false;
    }
#endif