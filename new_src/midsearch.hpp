﻿#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include <future>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "transpose_table.hpp"
#include "endsearch.hpp"
#include "move_ordering.hpp"
#if USE_MULTI_THREAD
    #include "thread_pool.hpp"
    #include "ybwc.hpp"
#endif
#if USE_LOG
    #include "log.hpp"
#endif

using namespace std;

int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped);
int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth, bool skipped);

inline bool mpc_higher(Search *search, int beta, int depth){
    int bound = beta + ceil(search->mpct * mpcsd[search->board.phase()][depth - MID_MPC_MIN_DEPTH]);
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
            res = nega_alpha_ordering_nomemo(search, bound - 1, bound, mpcd[depth], false) >= bound;
            break;
    }
    return res;
}

inline bool mpc_lower(Search *search, int alpha, int depth){
    int bound = alpha - ceil(search->mpct * mpcsd[search->board.phase()][depth - MID_MPC_MIN_DEPTH]);
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
            res = nega_alpha_ordering_nomemo(search, bound, bound + 1, mpcd[depth], false) <= bound;
            break;
    }
    return res;
}

int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped){
    ++(search->n_nodes);
    int g, v = -INF;
    uint64_t legal = search->board.get_legal();
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_eval1(search, -beta, -alpha, true);
        search->board.pass();
        alpha = max(alpha, v);
        return v;
    }
    Flip flip;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &search->board, cell);
        search->board.move(&flip);
            ++(search->n_nodes);
            g = -mid_evaluate(&search->board);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}

int nega_alpha(Search *search, int alpha, int beta, int depth, bool skipped){
    if (!global_searching)
        return SCORE_UNDEFINED;
    ++(search->n_nodes);
    if (depth == 0)
        return mid_evaluate(&search->board);
    if (depth == 1)
        return nega_alpha_eval1(search, alpha, beta, false);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    int g, v = -INF;
    uint64_t legal = search->board.get_legal();
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha(search, -beta, -alpha, depth, true);
        search->board.pass();
        alpha = max(alpha, v);
        return v;
    }
    Flip flip;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &search->board, cell);
        search->board.move(&flip);
            g = -nega_alpha(search, -beta, -alpha, depth - 1, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}

int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth, bool skipped){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped);
    if (depth == 1)
        return nega_alpha_eval1(search, alpha, beta, skipped);
    if (depth == 0)
        return mid_evaluate(&search->board);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    #if USE_MID_MPC
        if (MID_MPC_MIN_DEPTH <= depth && depth <= MID_MPC_MAX_DEPTH && search->use_mpc){
            if (mpc_higher(search, beta, depth))
                return beta;
            if (mpc_lower(search, alpha, depth))
                return alpha;
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth, true);
        search->board.pass();
        return v;
    }
    const int canput = pop_count_ull(legal);
    vector<Flip> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
        calc_flip(&move_list[idx++], &search->board, cell);
    move_ordering(search, move_list, depth, alpha, beta, false);
    for (const Flip &flip: move_list){
        search->board.move(&flip);
            g = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth - 1, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}

int nega_alpha_ordering(Search *search, int alpha, int beta, int depth, bool skipped, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta, skipped, searching);
    if (!is_end_search && depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped);
    if (!is_end_search && depth == 1)
        return nega_alpha_eval1(search, alpha, beta, false);
    if (!is_end_search && depth == 0)
        return mid_evaluate(&search->board);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        parent_transpose_table.get(search->tt_parent_idx, &search->board, hash_code, &l, &u);
        if (u == l)
            return u;
        if (l >= beta)
            return l;
        if (alpha >= u)
            return u;
        alpha = max(alpha, l);
        beta = min(beta, u);
    #endif
    #if USE_MID_MPC
        if (search->use_mpc){
            if (!is_end_search && MID_MPC_MIN_DEPTH <= depth && depth <= MID_MPC_MAX_DEPTH){
                if (mpc_higher(search, beta, depth))
                    return beta;
                if (mpc_lower(search, alpha, depth))
                    return alpha;
            } else if (is_end_search && END_MPC_MIN_DEPTH <= depth && depth <= END_MPC_MAX_DEPTH){
                if (mpc_end_higher(search, beta))
                    return beta;
                if (mpc_end_lower(search, alpha))
                    return alpha;
            }
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_ordering(search, -beta, -alpha, depth, true, is_end_search, searching);
        search->board.pass();
        return v;
    }
    const int canput = pop_count_ull(legal);
    vector<Flip> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
        calc_flip(&move_list[idx++], &search->board, cell);
    move_ordering(search, move_list, depth, alpha, beta, is_end_search);
    int best_move = -1;
    #if USE_MULTI_THREAD
        int pv_idx = 0, split_count = 0;
        vector<future<pair<int, uint64_t>>> parallel_tasks;
        bool n_searching = true;
        for (const Mobility &mob: move_list){
            if (!(*searching))
                break;
            if (ybwc_split(search, &mob, -beta, -alpha, depth - 1, is_end_search, &n_searching, flip.pos, pv_idx++, canput, split_count, parallel_tasks)){
                ++split_count;
            } else{
                search->board.move(&mob);
                    g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1, is_end_search, searching);
                search->board.undo(&mob);
                if (*searching){
                    alpha = max(alpha, g);
                    if (v < g){
                        v = g;
                        best_move = flip.pos;
                        //child_transpose_table.reg(search->tt_child_idx, &search->board, hash_code, flip.pos, g);
                    }
                    if (beta <= alpha)
                        break;
                }
            }
        }
        if (split_count){
            if (beta <= alpha || !(*searching)){
                n_searching = false;
                ybwc_wait_strict(search, parallel_tasks);
            } else{
                g = ybwc_wait_strict(search, parallel_tasks);
                alpha = max(alpha, g);
                v = max(v, g);
            }
        }
    #else
        for (const Flip &flip: move_list){
            search->board.move(&flip);
                g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1, false, is_end_search, searching);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (v < g){
                v = g;
                best_move = flip.pos;
                //child_transpose_table.reg(search->tt_child_idx, &search->board, hash_code, flip.pos, g);
            }
            if (beta <= alpha)
                break;
        }
    #endif
    child_transpose_table.reg(search->tt_child_idx, &search->board, hash_code, best_move, v);
    #if USE_MID_TC
        if (beta <= v)
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, v, u);
        else if (v <= alpha)
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, l, v);
        else
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, v, v);
    #endif
    return v;
}

int nega_scout(Search *search, int alpha, int beta, int depth, bool skipped, bool is_end_search){
    if (!global_searching)
        return -INF;
    //if (is_end_search && depth <= MID_TO_END_DEPTH)
    //    return nega_scout_end(search, alpha, beta);
    //if (!is_end_search && depth <= MID_FAST_DEPTH)
    //    return nega_alpha(search, alpha, beta, depth);
    if (!is_end_search && depth == 1)
        return nega_alpha_eval1(search, alpha, beta, false);
    if (!is_end_search && depth == 0)
        return mid_evaluate(&search->board);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        parent_transpose_table.get(search->tt_parent_idx, &search->board, hash_code, &l, &u);
        if (u == l)
            return u;
        if (l >= beta)
            return l;
        if (alpha >= u)
            return u;
        alpha = max(alpha, l);
        beta = min(beta, u);
    #endif
    #if USE_MID_MPC
        if (search->use_mpc){
            if (!is_end_search && MID_MPC_MIN_DEPTH <= depth && depth <= MID_MPC_MAX_DEPTH){
                if (mpc_higher(search, beta, depth))
                    return beta;
                if (mpc_lower(search, alpha, depth))
                    return alpha;
            } else if (is_end_search && END_MPC_MIN_DEPTH <= depth && depth <= END_MPC_MAX_DEPTH){
                if (mpc_end_higher(search, beta))
                    return beta;
                if (mpc_end_lower(search, alpha))
                    return alpha;
            }
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_scout(search, -beta, -alpha, depth, true, is_end_search);
        search->board.pass();
        return v;
    }
    const int canput = pop_count_ull(legal);
    vector<Flip> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
        calc_flip(&move_list[idx++], &search->board, cell);
    move_ordering(search, move_list, depth, alpha, beta, is_end_search);
    bool searching = true;
    int best_move = -1;
    for (const Flip &flip: move_list){
        search->board.move(&flip);
            if (v == -INF)
                g = -nega_scout(search, -beta, -alpha, depth - 1, false, is_end_search);
            else{
                g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, is_end_search, &searching);
                if (alpha < g)
                    g = -nega_scout(search, -beta, -g, depth - 1, false, is_end_search);
            }
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (v < g){
            v = g;
            //child_transpose_table.reg(search->tt_child_idx, &search->board, hash_code, flip.pos, g);
            best_move = flip.pos;
        }
        if (beta <= alpha)
            break;
    }
    child_transpose_table.reg(search->tt_child_idx, &search->board, hash_code, best_move, v);
    #if USE_MID_TC
        if (beta <= v)
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, v, u);
        else if (v <= alpha)
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, l, v);
        else
            parent_transpose_table.reg(search->tt_parent_idx, &search->board, hash_code, v, v);
    #endif
    return v;
}


int mtd(Search *search, int l, int u, int g, int depth, bool is_end_search){
    int beta;
    bool searching = true;
    g = max(l, min(u, g));
    while (u > l){
        beta = max(l + 1, g);
        g = nega_alpha_ordering(search, beta - 1, beta, depth, false, is_end_search, &searching);
        if (g < beta)
            u = g;
        else
            l = g;
    }
    return g;
}

/*
int mtd_end(Search *search, int l, int u, int g, int depth, bool is_end_search){
    int beta;
    l /= 2;
    u /= 2;
    g = max(l, min(u, g / 2));
    while (u > l){
        beta = max(l + 1, g);
        g = nega_alpha_ordering(search, beta * 2 - 2, beta * 2, depth, is_end_search) / 2;
        if (g < beta)
            u = g;
        else
            l = g;
    }
    return g * 2;
}
*/
