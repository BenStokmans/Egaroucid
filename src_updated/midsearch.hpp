﻿#pragma once
#include <iostream>
#include <algorithm>
#include <vector>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "transpose_table.hpp"
#include "endsearch.hpp"
#if USE_MULTI_THREAD
    #include "thread_pool.hpp"
    #include "ybwc.hpp"
#endif

using namespace std;

int nega_alpha(Search *search, int alpha, int beta, int depth);
inline bool mpc_higher(Search *search, int beta, int depth);
inline bool mpc_lower(Search *search, int alpha, int depth);

int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth){
    if (!global_searching)
        return -SCORE_UNDEFINED;
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth);
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
    unsigned long long legal = search->board.mobility_ull();
    int g, v = -INF;
    if (legal == 0){
        if (search->skipped)
            return end_evaluate(&search->board);
        search->pass();
            v = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth);
        search->undo_pass();
        alpha = max(alpha, v);
        return v;
    }
    const int canput = pop_count_ull(legal);
    vector<Mobility> move_list;
    for (const int &cell: search->vacant_list){
        if (1 & (legal >> cell))
            move_list.emplace_back(calc_flip(&search->board, cell));
    }
    move_ordering(search, move_list);
    for (const Mobility &mob: move_list){
        search->board.move(&mob);
            g = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth);
        search->board.undo(&mob);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    return v;
}

inline bool mpc_higher(Search *search, int beta, int depth){
    int bound = beta + ceil(search->mpct * mpcsd[search->board.phase()][depth - MID_MPC_MIN_DEPTH]);
    if (bound > HW2)
        bound = HW2; //return false;
    return nega_alpha_ordering_nomemo(search, bound - 1, bound, mpcd[depth]) >= bound;
}

inline bool mpc_lower(Search *search, int alpha, int depth){
    int bound = alpha - ceil(search->mpct * mpcsd[search->board.phase()][depth - MID_MPC_MIN_DEPTH]);
    if (bound < -HW2)
        bound = -HW2; //return false;
    return nega_alpha_ordering_nomemo(search, bound, bound + 1, mpcd[depth]) <= bound;
}

int nega_alpha(Search *search, int alpha, int beta, int depth){
    if (!global_searching)
        return SCORE_UNDEFINED;
    ++(search->n_nodes);
    if (depth == 0)
        return mid_evaluate(&search->board);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    int g, v = -INF;
    unsigned long long legal = search->board.mobility_ull();
    if (legal == 0){
        if (search->skipped)
            return end_evaluate(&search->board);
        search->pass();
            v = -nega_alpha(search, -beta, -alpha, depth);
        search->undo_pass();
        alpha = max(alpha, v);
        return v;
    }
    search->skipped = false;
    Mobility mob;
    for (const int &cell: search->vacant_list){
        if (1 & (legal >> cell)){
            calc_flip(&mob, &search->board, cell);
            search->board.move(&mob);
                g = -nega_alpha(search, -beta, -alpha, depth - 1);
            search->board.undo(&mob);
            alpha = max(alpha, g);
            v = max(v, g);
            if (beta <= alpha)
                break;
        }
    }
    return v;
}

int nega_alpha_ordering(Search *search, int alpha, int beta, int depth){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (depth >= HW2 - search->board.n && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta);
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    int hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        parent_transpose_table.get(&search->board, hash_code, &l, &u);
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
            if (depth < HW2 - search->board.n && MID_MPC_MIN_DEPTH <= depth && depth <= MID_MPC_MAX_DEPTH){
                if (mpc_higher(search, beta, depth)){
                    #if USE_MID_TC
                        if (l < beta)
                            parent_transpose_table.reg(&search->board, hash_code, beta, u);
                    #endif
                    return beta;
                }
                if (mpc_lower(search, alpha, depth)){
                    #if USE_MID_TC
                        if (alpha < u)
                            parent_transpose_table.reg(&search->board, hash_code, l, alpha);
                    #endif
                    return alpha;
                }
            } else if (depth >= HW2 - search->board.n && END_MPC_MIN_DEPTH <= depth && depth <= END_MPC_MAX_DEPTH){
                int val = mid_evaluate(&search->board);
                if (mpc_end_higher(search, beta, depth, val)){
                    #if USE_MID_TC
                        if (l < beta)
                            parent_transpose_table.reg(&search->board, hash_code, beta, u);
                    #endif
                    return beta;
                }
                if (mpc_end_lower(search, alpha, depth, val)){
                    #if USE_MID_TC
                        if (alpha < u)
                            parent_transpose_table.reg(&search->board, hash_code, l, alpha);
                    #endif
                    return alpha;
                }
            }
        }
    #endif
    unsigned long long legal = search->board.mobility_ull();
    int g, v = -INF;
    if (legal == 0){
        if (search->skipped)
            return end_evaluate(&search->board);
        search->pass();
            v = -nega_alpha_ordering(search, -beta, -alpha, depth);
        search->undo_pass();
        return v;
    }
    search->skipped = false;
    const int canput = pop_count_ull(legal);
    vector<Mobility> move_list;
    for (const int &cell: search->vacant_list){
        if (1 & (legal >> cell))
            move_list.emplace_back(calc_flip(&search->board, cell));
    }
    move_ordering(search, move_list);
    #if USE_MULTI_THREAD
        int pv_idx = 0, split_count = 0;
        vector<future<pair<int, unsigned long long>>> parallel_tasks;
        for (const Mobility &mob: move_list){
            search->board.move(&mob);
                if (ybwc_split(search, -beta, -alpha, depth - 1, mob.pos, pv_idx, canput, split_count, parallel_tasks)){
                    search->board.undo(&mob);
                    ++split_count;
                } else{
                    g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1);
                    search->board.undo(&mob);
                    child_transpose_table.reg(&search->board, hash_code, mob.pos, g);
                    alpha = max(alpha, g);
                    v = max(v, g);
                    if (beta <= alpha)
                        break;
                }
            ++pv_idx;
        }
        if (split_count){
            g = ybwc_wait(search, parallel_tasks);
            alpha = max(alpha, g);
            v = max(v, g);
        }
    #else
        for (const Mobility &mob: move_list){
            search->board.move(&mob);
                g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1);
            search->board.undo(&mob);
            child_transpose_table.reg(&search->board, hash_code, mob.pos, g);
            alpha = max(alpha, g);
            v = max(v, g);
            if (beta <= alpha)
                break;
        }
    #endif
    #if USE_MID_TC
        if (beta <= v)
            parent_transpose_table.reg(&search->board, hash_code, v, u);
        else if (v <= alpha)
            parent_transpose_table.reg(&search->board, hash_code, l, v);
        else
            parent_transpose_table.reg(&search->board, hash_code, v, v);
    #endif
    return v;
}

int nega_scout(Search *search, int alpha, int beta, int depth){
    if (!global_searching)
        return -INF;
    if (depth >= HW2 - search->board.n && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta);
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    int hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        parent_transpose_table.get(&search->board, hash_code, &l, &u);
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
            if (depth < HW2 - search->board.n && MID_MPC_MIN_DEPTH <= depth && depth <= MID_MPC_MAX_DEPTH){
                if (mpc_higher(search, beta, depth)){
                    #if USE_MID_TC
                        if (l < beta)
                            parent_transpose_table.reg(&search->board, hash_code, beta, u);
                    #endif
                    return beta;
                }
                if (mpc_lower(search, alpha, depth)){
                    #if USE_MID_TC
                        if (alpha < u)
                            parent_transpose_table.reg(&search->board, hash_code, l, alpha);
                    #endif
                    return alpha;
                }
            } else if (depth >= HW2 - search->board.n && END_MPC_MIN_DEPTH <= depth && depth <= END_MPC_MAX_DEPTH){
                int val = mid_evaluate(&search->board);
                if (mpc_end_higher(search, beta, depth, val)){
                    #if USE_MID_TC
                        if (l < beta)
                            parent_transpose_table.reg(&search->board, hash_code, beta, u);
                    #endif
                    return beta;
                }
                if (mpc_end_lower(search, alpha, depth, val)){
                    #if USE_MID_TC
                        if (alpha < u)
                            parent_transpose_table.reg(&search->board, hash_code, l, alpha);
                    #endif
                    return alpha;
                }
            }
        }
    #endif
    unsigned long long legal = search->board.mobility_ull();
    int g, v = -INF;
    if (legal == 0){
        if (search->skipped)
            return end_evaluate(&search->board);
        search->pass();
            v = -nega_scout(search, -beta, -alpha, depth);
        search->undo_pass();
        return v;
    }
    search->skipped = false;
    const int canput = pop_count_ull(legal);
    vector<Mobility> move_list;
    for (const int &cell: search->vacant_list){
        if (1 & (legal >> cell))
            move_list.emplace_back(calc_flip(&search->board, cell));
    }
    move_ordering(search, move_list);
    for (const Mobility &mob: move_list){
        search->board.move(&mob);
            if (v == -INF)
                g = -nega_scout(search, -beta, -alpha, depth - 1);
            else{
                g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1);
                if (alpha < g)
                    g = -nega_scout(search, -beta, -g, depth - 1);
            }
        search->board.undo(&mob);
        child_transpose_table.reg(&search->board, hash_code, mob.pos, g);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    #if USE_MID_TC
        if (beta <= v)
            parent_transpose_table.reg(&search->board, hash_code, v, u);
        else if (v <= alpha)
            parent_transpose_table.reg(&search->board, hash_code, l, v);
        else
            parent_transpose_table.reg(&search->board, hash_code, v, v);
    #endif
    return v;
}


int mtd(Search *search, int l, int u, int g, int depth){
    int beta;
    g = max(l, min(u, g));
    while (u > l){
        beta = max(l + 1, g);
        g = nega_alpha_ordering(search, beta - 1, beta, depth);
        if (g < beta)
            u = g;
        else
            l = g;
    }
    return g;
}

int mtd_end(Search *search, int l, int u, int depth){
    int g, beta, ll, uu;
    l /= 2;
    u /= 2;
    parent_transpose_table.get(&search->board, search->board.hash() & TRANSPOSE_TABLE_MASK, &ll, &uu);
    if (ll != -INF)
        g = ll / 2;
    else if (uu != INF)
        g = uu / 2;
    else
        g = nega_alpha(search, l, u, 5) / 2;
    while (u > l){
        beta = max(l + 1, g);
        g = nega_alpha_ordering(search, beta * 2 - 2, beta * 2, depth) / 2;
        if (g < beta)
            u = g;
        else
            l = g;
    }
    return g * 2;
}

inline Search_result tree_search(Board b, int max_depth, bool use_mpc, double mpct, const vector<int> vacant_lst){
    long long strt = tim();
    int hash_code = b.hash() & TRANSPOSE_TABLE_MASK;
    Search search;
    search.board = b;
    search.skipped = false;
    search.use_mpc = use_mpc;
    search.mpct = mpct;
    search.vacant_list = vacant_lst;
    search.n_nodes = 0;
    unsigned long long legal = b.mobility_ull();
    const int canput = pop_count_ull(legal);
    vector<Mobility> move_list;
    for (const int &cell: search.vacant_list){
        if (1 & (legal >> cell))
            move_list.emplace_back(calc_flip(&search.board, cell));
    }
    Search_result res;
    int alpha, beta, g, former_alpha = -INF, l, u;
    vector<int> expected_values;
    for (int i = 0; i < canput; ++i)
        expected_values.emplace_back(0);
    if (b.n + max_depth < HW2){
        for (int depth = min(16, max(0, max_depth - 5)); depth <= max_depth; ++depth){
            alpha = -HW2;
            beta = HW2;
            for (int i = 0; i < (int)move_list.size(); ++i){
                search.board.move(&move_list[i]);
                    parent_transpose_table.get(&search.board, search.board.hash() & TRANSPOSE_TABLE_MASK, &l, &u);
                search.board.undo(&move_list[i]);
                if (l != -INF)
                    expected_values[i] = l;
                else if (u != INF)
                    expected_values[i] = u;
            }
            parent_transpose_table.init();
            child_transpose_table.ready_next_search();
            move_ordering(&search, move_list);
            for (int i = 0; i < (int)move_list.size(); ++i){
                search.board.move(&move_list[i]);
                    g = -mtd(&search, -beta, -alpha, expected_values[i], depth - 1);
                search.board.undo(&move_list[i]);
                if (alpha < g){
                    child_transpose_table.reg(&search.board, hash_code, move_list[i].pos, g);
                    alpha = g;
                    res.policy = move_list[i].pos;
                }
            }
            if (depth == max_depth - 2)
                former_alpha = alpha;
            cerr << "midsearch time " << tim() - strt << " depth " << depth << " policy " << res.policy << " value " << alpha << " nodes " << search.n_nodes << " nps " << search.n_nodes * 1000 / max(1LL, tim() - strt) << endl;
        }
    } else{
        if (b.n >= 60 && false){

        } else{
            int depth = HW2 - b.n;
            vector<double> pre_search_mpcts;
            pre_search_mpcts.emplace_back(0.5);
            if (search.mpct > 1.6 || !search.use_mpc)
                pre_search_mpcts.emplace_back(1.0);
            if (search.mpct > 2.0 || !search.use_mpc)
                pre_search_mpcts.emplace_back(1.5);
            search.use_mpc = true;
            for (double pre_search_mpct: pre_search_mpcts){
                alpha = -HW2;
                beta = HW2;
                search.mpct = pre_search_mpct;
                for (int i = 0; i < (int)move_list.size(); ++i){
                    search.board.move(&move_list[i]);
                        parent_transpose_table.get(&search.board, search.board.hash() & TRANSPOSE_TABLE_MASK, &l, &u);
                    search.board.undo(&move_list[i]);
                    if (l != -INF)
                        expected_values[i] = l;
                    else if (u != INF)
                        expected_values[i] = u;
                }
                parent_transpose_table.init();
                child_transpose_table.ready_next_search();
                move_ordering(&search, move_list);
                for (int i = 0; i < (int)move_list.size(); ++i){
                    search.board.move(&move_list[i]);
                        g = -mtd(&search, -beta, -alpha, expected_values[i], depth - 1);
                    search.board.undo(&move_list[i]);
                    if (alpha < g){
                        child_transpose_table.reg(&search.board, hash_code, move_list[i].pos, g);
                        alpha = g;
                        res.policy = move_list[i].pos;
                    }
                }
                cerr << "endsearch time " << tim() - strt << " mpct " << search.mpct << " policy " << res.policy << " value " << alpha << " nodes " << search.n_nodes << " nps " << search.n_nodes * 1000 / max(1LL, tim() - strt) << endl;
            }
            alpha = -HW2;
            beta = HW2;
            search.use_mpc = use_mpc;
            search.mpct = mpct;
            for (int i = 0; i < (int)move_list.size(); ++i){
                search.board.move(&move_list[i]);
                    parent_transpose_table.get(&search.board, search.board.hash() & TRANSPOSE_TABLE_MASK, &l, &u);
                search.board.undo(&move_list[i]);
                if (l != -INF)
                    expected_values[i] = -l;
                else if (u != INF)
                    expected_values[i] = -u;
            }
            parent_transpose_table.init();
            child_transpose_table.ready_next_search();

            int best_moves[3];
            child_transpose_table.get_prev(&search.board, search.board.hash() & TRANSPOSE_TABLE_MASK, best_moves);
            cerr << "best moves ";
            for (int i = 0; i < 3; ++i)
                cerr << best_moves[i] << " ";
            cerr << endl;

            move_ordering(&search, move_list);
            for (int i = 0; i < (int)move_list.size(); ++i){
                search.board.move(&move_list[i]);
                    g = -mtd(&search, -beta, -alpha, expected_values[i], depth - 1);
                search.board.undo(&move_list[i]);
                if (alpha < g){
                    child_transpose_table.reg(&search.board, hash_code, move_list[i].pos, g);
                    alpha = g;
                    res.policy = move_list[i].pos;
                }
            }
            cerr << "endsearch time " << tim() - strt << " mpct " << search.mpct << " policy " << res.policy << " value " << alpha << " nodes " << search.n_nodes << " nps " << search.n_nodes * 1000 / max(1LL, tim() - strt) << endl;
        }
    }
    res.depth = max_depth;
    res.nps = search.n_nodes * 1000 / max(1LL, tim() - strt);
    if (former_alpha != -INF)
        res.value = (former_alpha + alpha) / 2;
    else
        res.value = alpha;
    return res;
}

inline Search_result tree_search_value(Board b, int max_depth, bool use_mpc, double mpct, const vector<int> vacant_lst){
    long long strt = tim();
    Search search;
    search.board = b;
    search.skipped = false;
    search.use_mpc = use_mpc;
    search.mpct = mpct;
    search.vacant_list = vacant_lst;
    search.n_nodes = 0;
    int g, former_g = 0;
    for (int depth = min(16, max(0, max_depth - 5)); depth <= max_depth - 1; ++depth){
        parent_transpose_table.init();
        child_transpose_table.ready_next_search();
        g = -mtd(&search, -HW2, HW2, -former_g, depth);
        if (depth == max_depth - 2)
            former_g = g;
    }
    Search_result res;
    res.depth = max_depth;
    res.nps = search.n_nodes * 1000 / max(1LL, tim() - strt);
    if (former_g != -INF)
        res.value = (former_g + g) / 2;
    else
        res.value = g;
    return res;
}
