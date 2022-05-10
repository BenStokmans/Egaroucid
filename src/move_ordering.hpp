#pragma once
#include <iostream>
#include <vector>
#include "common.hpp"
#include "board.hpp"
#include "search.hpp"
#include "transpose_table.hpp"
#if USE_CUDA
    #include "cuda_midsearch.hpp"
#else
    #include "midsearch.hpp"
#endif
#include "probcut.hpp"

#define N_MOVE_ORDERING_PATTERNS 10
#define MAX_MOVE_ORDERING_EVALUATE_IDX 65536
#define MOVE_ORDERING_PHASE_DIV 10
#define N_MOVE_ORDERING_PHASE 6

#define W_BEST_MOVE 900000000

#define W_WIPEOUT 1000000000

#define W_VALUE 8
#define W_VALUE_SHALLOW 4
#define W_STABILITY 4
#define W_MOBILITY 16
//#define W_FLIP_INSIDE 4
//#define W_SURROUND 4
#define W_PARITY1 2
#define W_PARITY2 4
#define W_PARITY3 8

#define MOVE_ORDERING_VALUE_OFFSET 14

#define W_END_MOBILITY 64
#define W_END_PARITY 14

int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped, const bool *searching);
int nega_alpha(Search *search, int alpha, int beta, int depth, bool skipped, const bool *searching);
int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, const bool *searching);
int nega_scout(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching);

inline uint64_t calc_legal_flip_inside(uint64_t player, uint64_t opponent){
    uint64_t stones = ~(player | opponent);
    uint64_t hmask = stones & 0x7E7E7E7E7E7E7E7EULL;
    uint64_t vmask = stones & 0x00FFFFFFFFFFFF00ULL;
    uint64_t hvmask = stones & 0x007E7E7E7E7E7E00ULL;
    uint64_t outside_stones_mask =  (hmask << 1) | (hmask >> 1) | 
                                    (vmask << HW) | (vmask >> HW) | 
                                    (hvmask << HW_M1) | (hvmask >> HW_M1) | 
                                    (hvmask << HW_P1) | (hvmask >> HW_P1);
    uint64_t legal = calc_legal(player, opponent & ~outside_stones_mask) & ~stones;
    if (legal == 0ULL)
        return 0ULL;
    return legal & ~calc_legal(player, opponent & outside_stones_mask);
}

inline bool is_flip_inside(Flip *flip, uint64_t stones){
    uint64_t hmask = flip->flip & 0x7E7E7E7E7E7E7E7EULL;
    uint64_t vmask = flip->flip & 0x00FFFFFFFFFFFF00ULL;
    uint64_t hvmask = flip->flip & 0x007E7E7E7E7E7E00ULL;
    uint64_t connect = (hmask << 1) | (hmask >> 1) | (vmask << HW) | (vmask >> HW) | (hvmask << HW_M1) | (hvmask >> HW_M1) | (hvmask << HW_P1) | (hvmask >> HW_P1);
    connect &= ~flip->flip;
    return (connect & ~(stones | (1ULL << flip->pos))) == 0ULL;
}

inline bool is_disturb_opponent_flip_inside(Flip *flip, Board *board){

}

inline bool is_create_my_flip_inside(Flip *flip, Board *board){

}

inline bool is_create_opponent_flip_inside(Flip *flip, Board *board){

}

inline void move_sort_top(vector<Flip> &move_list, int best_idx){
    if (best_idx != 0)
        swap(move_list[best_idx], move_list[0]);
}

bool cmp_move_ordering(Flip &a, Flip &b){
    return a.value > b.value;
}

inline void move_evaluate(Search *search, Flip *flip, const int alpha, const int beta, const int depth, const bool *searching, uint64_t stones){
    if (flip->flip == search->board.opponent)
        flip->value = W_WIPEOUT;
    else{
        flip->value = cell_weight[flip->pos];
        if (search->board.parity & cell_div4[flip->pos]){
            if (search->board.n < 34)
                flip->value += W_PARITY1;
            else if (search->board.n < 44)
                flip->value += W_PARITY2;
            else
                flip->value += W_PARITY3;
        }
        if (depth < 0){
            search->board.move(flip);
                //flip->value += -calc_surround(search->board.opponent, ~(search->board.player | search->board.opponent)) * W_SURROUND;
                flip->value += calc_stability_edge_player(search->board.opponent, search->board.player) * W_STABILITY;
                flip->n_legal = search->board.get_legal();
                flip->value += -pop_count_ull(flip->n_legal) * W_MOBILITY;
            search->board.undo(flip);
        } else{
            eval_move(search, flip);
            search->board.move(flip);
                //flip->value += -calc_surround(search->board.opponent, ~(search->board.player | search->board.opponent)) * W_SURROUND;
                flip->value += calc_stability_edge_player(search->board.opponent, search->board.player) * W_STABILITY;
                flip->n_legal = search->board.get_legal();
                flip->value += -pop_count_ull(flip->n_legal) * W_MOBILITY;
                if (depth >= 0){
                    switch(depth){
                        case 0:
                            flip->value += (HW2 - value_to_score_int(mid_evaluate_diff(search, searching))) * W_VALUE_SHALLOW;
                            break;
                        case 1:
                            flip->value += (HW2 - value_to_score_int(nega_alpha_eval1(search, alpha, beta, false, searching))) * W_VALUE;
                            break;
                        default:
                            if (depth <= MID_FAST_DEPTH)
                                flip->value += (HW2 - value_to_score_int(nega_alpha(search, alpha, beta, depth, false, searching))) * W_VALUE;
                            else{
                                bool use_mpc = search->use_mpc;
                                search->use_mpc = false;
                                    flip->value += (HW2 - value_to_score_int(nega_alpha_ordering_nomemo(search, alpha, beta, depth, false, flip->n_legal, searching))) * W_VALUE;
                                search->use_mpc = use_mpc;
                            }
                            break;
                    }
                }
            search->board.undo(flip);
            eval_undo(search, flip);
        }
    }
}

/* old version
inline void move_evaluate(Search *search, Flip *flip, const int alpha, const int beta, const int depth, const bool *searching, uint64_t stones){
    if (flip->flip == search->board.opponent)
        flip->value = W_WIPEOUT;
    else{
        flip->value = cell_weight[flip->pos];
        if (search->board.parity & cell_div4[flip->pos]){
            if (search->board.n < 34)
                flip->value += W_PARITY1;
            else if (search->board.n < 44)
                flip->value += W_PARITY2;
            else
                flip->value += W_PARITY3;
        }
        if (depth < 0){
            search->board.move(flip);
                //flip->value += -calc_surround(search->board.opponent, ~(search->board.player | search->board.opponent)) * W_SURROUND;
                flip->value += calc_stability_edge_player(search->board.opponent, search->board.player) * W_STABILITY;
                flip->n_legal = search->board.get_legal();
                flip->value += -pop_count_ull(flip->n_legal) * W_MOBILITY;
            search->board.undo(flip);
        } else{
            eval_move(search, flip);
            search->board.move(flip);
                //flip->value += -calc_surround(search->board.opponent, ~(search->board.player | search->board.opponent)) * W_SURROUND;
                flip->value += calc_stability_edge_player(search->board.opponent, search->board.player) * W_STABILITY;
                flip->n_legal = search->board.get_legal();
                flip->value += -pop_count_ull(flip->n_legal) * W_MOBILITY;
                if (depth >= 0){
                    switch(depth){
                        case 0:
                            flip->value += (HW2 - value_to_score_int(mid_evaluate_diff(search, searching))) * W_VALUE_SHALLOW;
                            break;
                        case 1:
                            flip->value += (HW2 - value_to_score_int(nega_alpha_eval1(search, alpha, beta, false, searching))) * W_VALUE;
                            break;
                        default:
                            if (depth <= MID_FAST_DEPTH)
                                flip->value += (HW2 - value_to_score_int(nega_alpha(search, alpha, beta, depth, false, searching))) * W_VALUE;
                            else{
                                bool use_mpc = search->use_mpc;
                                search->use_mpc = false;
                                    flip->value += (HW2 - value_to_score_int(nega_alpha_ordering_nomemo(search, alpha, beta, depth, false, flip->n_legal, searching))) * W_VALUE;
                                search->use_mpc = use_mpc;
                            }
                            break;
                    }
                }
            search->board.undo(flip);
            eval_undo(search, flip);
        }
    }
}
*/

inline void move_evaluate_fast_first(Search *search, Flip *flip){
    if (flip->flip == search->board.opponent)
        flip->value = W_WIPEOUT;
    else{
        flip->value = cell_weight[flip->pos];
        if (search->board.parity & cell_div4[flip->pos]){
            if (search->board.n < 34)
                flip->value += W_PARITY1;
            else if (search->board.n < 44)
                flip->value += W_PARITY2;
            else
                flip->value += W_PARITY3;
        }
        search->board.move(flip);
            flip->value += calc_stability_edge_player(search->board.opponent, search->board.player) * W_STABILITY;
            flip->n_legal = search->board.get_legal();
            flip->value += -pop_count_ull(flip->n_legal) * W_MOBILITY;
        search->board.undo(flip);
    }
}

inline void move_ordering(Search *search, vector<Flip> &move_list, int depth, int alpha, int beta, bool is_end_search, const bool *searching){
    if (move_list.size() < 2)
        return;
    int eval_alpha = -min(SCORE_MAX, beta + MOVE_ORDERING_VALUE_OFFSET);
    int eval_beta = -max(-SCORE_MAX, alpha - MOVE_ORDERING_VALUE_OFFSET);
    int eval_depth = depth >> 3;
    if (depth >= 22)
        ++eval_depth;
    if (depth >= 26)
        ++eval_depth;
    //eval_depth = max(0, eval_depth);
    uint64_t stones = search->board.player | search->board.opponent;
    for (Flip &flip: move_list)
        move_evaluate(search, &flip, eval_alpha, eval_beta, eval_depth, searching, stones);
    sort(move_list.begin(), move_list.end(), cmp_move_ordering);
}
/*
inline void move_evaluate_fast_first(Search *search, Flip *flip, const int best_move){
    flip->value = 0;
    if (flip->pos == best_move)
        flip->value = W_BEST_MOVE;
    else{
        if (search->board.parity & cell_div4[flip->pos])
            flip->value += W_END_PARITY;
        search->board.move(flip);
            flip->n_legal = search->board.get_legal();
            flip->value -= pop_count_ull(flip->n_legal) * W_END_MOBILITY;
        search->board.undo(flip);
    }
}

inline void move_evaluate_fast_first(Search *search, Flip *flip){
    flip->value = 0;
    if (search->board.parity & cell_div4[flip->pos])
        flip->value += W_END_PARITY;
    search->board.move(flip);
        flip->n_legal = search->board.get_legal();
        flip->value -= pop_count_ull(flip->n_legal) * W_END_MOBILITY;
    search->board.undo(flip);
}
*/
inline void move_ordering_fast_first(Search *search, vector<Flip> &move_list){
    if (move_list.size() < 2)
        return;
    for (Flip &flip: move_list)
        move_evaluate_fast_first(search, &flip);
    //move_evaluate_fast_first(search, &flip);
    sort(move_list.begin(), move_list.end(), cmp_move_ordering);
}

