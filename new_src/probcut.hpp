#pragma once
#include <iostream>
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "midsearch.hpp"

using namespace std;

constexpr double mpc_params[7] = {
    0.12506284069441298, 0.003146851320878552, -3.129294209685033, 0.01836594544092185, 0.42200995669259966, -1.828870503078114, 5.687890822032203
};

constexpr int mpcd[61] = {
    0, 1, 0, 1, 2, 3, 2, 3, 2, 3, 
    4, 3, 4, 3, 4, 5, 4, 5, 6, 5, 
    6, 5, 6, 7, 6, 7, 8, 7, 8, 7, 
    8, 9, 8, 9, 10, 9, 10, 9, 10, 11, 
    10, 11, 12, 11, 12, 11, 12, 13, 12, 13, 
    14, 13, 14, 13, 14, 15, 14, 15, 16, 15,
    16
};

inline double probcut_sigma(int n_stones, int depth){
    double x = mpc_params[0] * (double)n_stones + mpc_params[1] * depth + mpc_params[2];
    //cerr << mpc_params[3] * x * x * x + mpc_params[4] * x * x + mpc_params[5] * x + mpc_params[6] << endl;
    return mpc_params[3] * x * x * x + mpc_params[4] * x * x + mpc_params[5] * x + mpc_params[6];
}

int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped);
int nega_alpha(Search *search, int alpha, int beta, int depth, bool skipped);
int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal);

inline bool mpc_higher(Search *search, int beta, int depth, uint64_t legal){
    int bound = beta + ceil(search->mpct * probcut_sigma(search->board.n, depth));
    if (bound > HW2)
        bound = HW2; //return false;
    bool res;
    switch(mpcd[depth]){
        case 0:
            res = mid_evaluate(&search->board) >= bound;
            break;
        case 1:
            res = nega_alpha_eval1(search, bound - 1, bound, false) >= bound;
            break;
        default:
            if (mpcd[depth] <= MID_FAST_DEPTH)
                res = nega_alpha(search, bound - 1, bound, mpcd[depth], false) >= bound;
            else{
                //double mpct = search->mpct;
                //search->mpct = 1.18;
                search->use_mpc = false;
                    res = nega_alpha_ordering_nomemo(search, bound - 1, bound, mpcd[depth], false, legal) >= bound;
                search->use_mpc = true;
                //search->mpct = mpct;
            }
            break;
    }
    return res;
}

inline bool mpc_lower(Search *search, int alpha, int depth, uint64_t legal){
    int bound = alpha - ceil(search->mpct * probcut_sigma(search->board.n, depth));
    if (bound < -HW2)
        bound = -HW2; //return false;
    bool res;
    switch(mpcd[depth]){
        case 0:
            res = mid_evaluate(&search->board) <= bound;
            break;
        case 1:
            res = nega_alpha_eval1(search, bound, bound + 1, false) <= bound;
            break;
        default:
            if (mpcd[depth] <= MID_FAST_DEPTH)
                res = nega_alpha(search, bound, bound + 1, mpcd[depth], false) <= bound;
            else{
                //double mpct = search->mpct;
                //search->mpct = 1.18;
                search->use_mpc = false;
                    res = nega_alpha_ordering_nomemo(search, bound, bound + 1, mpcd[depth], false, legal) <= bound;
                search->use_mpc = true;
                //search->mpct = mpct;
            }
            break;
    }
    return res;
}