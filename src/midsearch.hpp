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
#include "probcut.hpp"
#if USE_MULTI_THREAD
    #include "thread_pool.hpp"
    #include "ybwc.hpp"
#endif
#if USE_LOG
    #include "log.hpp"
#endif
#include "util.hpp"
#include "book.hpp"

using namespace std;

inline int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    ++(search->n_nodes);
    int g, v = -INF;
    uint64_t legal = search->board.get_legal();
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_eval1(search, -beta, -alpha, true, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    Flip flip;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &search->board, cell);
        eval_move(search, &flip);
        search->board.move(&flip);
            g = -mid_evaluate_diff(search);
        search->board.undo(&flip);
        eval_undo(search, &flip);
        ++(search->n_nodes);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}

int nega_alpha(Search *search, int alpha, int beta, int depth, bool skipped, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    ++(search->n_nodes);
    if (depth == 1)
        return nega_alpha_eval1(search, alpha, beta, skipped, searching);
    if (depth == 0)
        return mid_evaluate_diff(search);
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
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha(search, -beta, -alpha, depth, true, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    Flip flip;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &search->board, cell);
        eval_move(search, &flip);
        search->board.move(&flip);
            g = -nega_alpha(search, -beta, -alpha, depth - 1, false, searching);
        search->board.undo(&flip);
        eval_undo(search, &flip);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}

int nega_alpha_ordering_nomemo(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped, searching);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    int first_alpha = alpha;
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, false, &v, searching))
                return v;
        }
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, searching);
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, false, searching);
        for (const Flip &flip: move_list){
            eval_move(search, &flip);
            search->board.move(&flip);
                g = -nega_alpha_ordering_nomemo(search, -beta, -alpha, depth - 1, false, flip.n_legal, searching);
            search->board.undo(&flip);
            eval_undo(search, &flip);
            alpha = max(alpha, g);
            if (v < g){
                v = g;
                best_move = flip.pos;
            }
            if (beta <= alpha)
                break;
        }
    }
    //if (best_move != child_transpose_table.get(&search->board, hash_code))
    if (first_alpha < v)
        child_transpose_table.reg(&search->board, hash_code, best_move);
    return v;
}

int nega_alpha_ordering(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta, skipped, legal, searching);
    if (!is_end_search && depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped, searching);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        if (depth >= USE_PARENT_TT_DEPTH_THRESHOLD){
            parent_transpose_table.get(&search->board, hash_code, &l, &u);
            if (u == l)
                return u;
            if (beta <= l)
                return l;
            if (u <= alpha)
                return u;
            alpha = max(alpha, l);
            beta = min(beta, u);
        }
    #endif
    int first_alpha = alpha;
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_ordering(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED, is_end_search, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, is_end_search, &v, searching))
                return v;
        }
    #endif
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, is_end_search, searching);
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, is_end_search, searching);
        #if USE_MULTI_THREAD
            int pv_idx = 0, split_count = 0;
            if (best_move != TRANSPOSE_TABLE_UNDEFINED)
                pv_idx = 1;
            vector<future<Parallel_task>> parallel_tasks;
            bool n_searching = true;
            for (const Flip &flip: move_list){
                if (!(*searching))
                    break;
                eval_move(search, &flip);
                search->board.move(&flip);
                    if (ybwc_split_without_move(search, &flip, -beta, -alpha, depth - 1, flip.n_legal, is_end_search, &n_searching, flip.pos, pv_idx++, canput, split_count, parallel_tasks, move_list[0].value, move_list[move_list.size() - 1].value)){
                        ++split_count;
                    } else{
                        g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                        if (*searching){
                            alpha = max(alpha, g);
                            if (v < g){
                                v = g;
                                best_move = flip.pos;
                            }
                            if (beta <= alpha){
                                search->board.undo(&flip);
                                eval_undo(search, &flip);
                                break;
                            }
                            ybwc_get_end_tasks(search, parallel_tasks, &v, &best_move);
                            alpha = max(alpha, v);
                            if (beta <= alpha){
                                search->board.undo(&flip);
                                eval_undo(search, &flip);
                                break;
                            }
                        }
                    }
                search->board.undo(&flip);
                eval_undo(search, &flip);
            }
            if (split_count){
                if (beta <= alpha || !(*searching)){
                    n_searching = false;
                    ybwc_wait_all(search, parallel_tasks);
                } else{
                    ybwc_wait_all(search, parallel_tasks, &v, &best_move);
                    alpha = max(alpha, v);
                }
            }
        #else
            for (const Flip &flip: move_list){
                eval_move(search, &flip);
                search->board.move(&flip);
                    g = -nega_alpha_ordering(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                search->board.undo(&flip);
                eval_undo(search, &flip);
                alpha = max(alpha, g);
                if (v < g){
                    v = g;
                    best_move = flip.pos;
                }
                if (beta <= alpha)
                    break;
            }
        #endif
    }
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}

int nega_scout(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta, skipped, legal, searching);
    if (!is_end_search && depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped, searching);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #elif USE_END_SC
        if (is_end_search){
            int stab_res = stability_cut(search, &alpha, &beta);
            if (stab_res != SCORE_UNDEFINED)
                return stab_res;
        }
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        if (depth >= USE_PARENT_TT_DEPTH_THRESHOLD){
            parent_transpose_table.get(&search->board, hash_code, &l, &u);
            if (u == l)
                return u;
            if (beta <= l)
                return l;
            if (u <= alpha)
                return u;
            alpha = max(alpha, l);
            beta = min(beta, u);
        }
    #endif
    int first_alpha = alpha;
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_scout(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED, is_end_search, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, is_end_search, &v, searching))
                return v;
        }
    #endif
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_scout(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, is_end_search, searching);
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            v = g;
            alpha = max(alpha, g);
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, is_end_search, searching);
        //bool searching = true;
        #if USE_MULTI_THREAD && false // UNDER CONSTRUCTION
            int pv_idx = 0, split_count = 0;
            if (best_move != TRANSPOSE_TABLE_UNDEFINED)
                pv_idx = 1;
            vector<future<pair<int, uint64_t>>> parallel_tasks;
            vector<Flip> parallel_flips;
            bool n_searching = true;
            int before_alpha = alpha;
            for (const Flip &flip: move_list){
                eval_move(search, &flip);
                search->board.move(&flip);
                    if (v == -INF){
                        g = -nega_scout(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search);
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = flip.pos;
                        }
                        if (beta <= alpha){
                            search->board.undo(&flip);
                            eval_undo(search, &flip);
                            break;
                        }
                    } else if (depth - 1 <= 11){
                        if (ybwc_split_without_move_negascout(search, &flip, -before_alpha - 1, -before_alpha, depth - 1, flip.n_legal, is_end_search, &n_searching, flip.pos, pv_idx, canput, split_count, parallel_tasks)){
                            parallel_flips.emplace_back(flip);
                            ++split_count;
                        } else{
                            g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                            if (alpha < g){
                                if (is_end_search){
                                    g = value_to_score_int(g);
                                    g -= g & 1;
                                    g = score_to_value(g);
                                }
                                g = -nega_scout(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search);
                            }
                            alpha = max(alpha, g);
                            if (v < g){
                                v = g;
                                best_move = flip.pos;
                            }
                            if (beta <= alpha){
                                search->board.undo(&flip);
                                eval_undo(search, &flip);
                                break;
                            }
                        }
                    } else{
                        g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                        if (alpha < g){
                            if (is_end_search){
                                g = value_to_score_int(g);
                                g -= g & 1;
                                g = score_to_value(g);
                            }
                            g = -nega_scout(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search);
                        }
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = flip.pos;
                        }
                        if (beta <= alpha){
                            search->board.undo(&flip);
                            eval_undo(search, &flip);
                            break;
                        }
                    }
                search->board.undo(&flip);
                eval_undo(search, &flip);
                ++pv_idx;
            }
            if (split_count){
                if (beta <= alpha){
                    n_searching = false;
                    ybwc_wait_all(search, parallel_tasks);
                } else{
                    g = ybwc_negascout_wait_all(search, parallel_tasks, parallel_flips,  before_alpha, alpha, beta, depth - 1, skipped, is_end_search, &best_move);
                    alpha = max(alpha, g);
                    v = max(v, g);
                }
            }
        #else
            for (const Flip &flip: move_list){
                eval_move(search, &flip);
                search->board.move(&flip);
                    if (v == -INF)
                        g = -nega_scout(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                    else{
                        g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                        if (alpha < g && g < beta)
                            g = -nega_scout(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search, searching);
                    }
                search->board.undo(&flip);
                eval_undo(search, &flip);
                if (v < g){
                    v = g;
                    best_move = flip.pos;
                    alpha = max(alpha, g);
                }
                if (beta <= alpha)
                    break;
            }
        #endif
    }
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}

pair<int, int> first_nega_scout(Search *search, int alpha, int beta, int depth, bool skipped, bool is_end_search, const bool is_main_search, int best_move){
    if (is_main_search)
        cerr << "start search depth " << depth << " [" << alpha << "," << beta << "] mpct " << search->mpct << endl;
    bool searching = true;
    ++(search->n_nodes);
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    uint64_t legal = search->board.get_legal();
    int first_alpha = alpha;
    int g, v = -INF;
    if (legal == 0ULL){
        pair<int, int> res;
        if (skipped){
            res.first = end_evaluate(&search->board);
            res.second = -1;
        } else{
            search->eval_feature_reversed ^= 1;
            search->board.pass();
                res = first_nega_scout(search, -beta, -alpha, depth, true, is_end_search, is_main_search, best_move);
            search->board.pass();
            search->eval_feature_reversed ^= 1;
            res.first = -res.first;
        }
        return res;
    }
    //int best_move = child_transpose_table.get(&search->board, hash_code);
    const int canput_all = pop_count_ull(legal);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_scout(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, is_end_search, &searching);
                if (is_main_search)
                    cerr << 1 << "/" << canput_all << " [" << alpha << "," << beta << "] mpct " << search->mpct << " " << idx_to_coord(best_move) << " value " << value_to_score_double(g) << endl;
                //search->board.print();
                //cerr << endl;
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            v = g;
            alpha = max(alpha, g);
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        int mobility_idx = (v == -INF) ? 1 : 2;
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, is_end_search, &searching);
        for (const Flip &flip: move_list){
            eval_move(search, &flip);
            search->board.move(&flip);
                /*
                g = book.get(&search->board);
                if (g < -SCORE_MAX || SCORE_MAX < g){
                    if (v == -INF)
                        g = -nega_scout(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                    else{
                        g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                        if (alpha < g)
                            g = -nega_scout(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search, &searching);
                    }
                }
                */
                if (v == -INF)
                    g = -nega_scout(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                else{
                    g = -nega_alpha_ordering(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, &searching);
                    if (alpha < g && g < beta)
                        g = -nega_scout(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search, &searching);
                }
                if (is_main_search){
                    if (g <= alpha)
                        cerr << mobility_idx << "/" << canput_all << " [" << alpha << "," << beta << "] mpct " << search->mpct << " " << idx_to_coord((int)flip.pos) << " value " << value_to_score_double(g) << " or lower" << endl;
                    else
                        cerr << mobility_idx << "/" << canput_all << " [" << alpha << "," << beta << "] mpct " << search->mpct << " " << idx_to_coord((int)flip.pos) << " value " << value_to_score_double(g) << endl;
                }
                ++mobility_idx;
                //search->board.print();
                //cerr << endl;
            search->board.undo(&flip);
            eval_undo(search, &flip);
            if (v < g){
                v = g;
                best_move = flip.pos;
                alpha = max(alpha, g);
            }
            if (beta <= alpha)
                break;
        }
    }
    //if (best_move != child_transpose_table.get(&search->board, hash_code))
    /*
    if (first_alpha < v)
        child_transpose_table.reg(&search->board, hash_code, best_move);
    #if USE_MID_TC
        if (depth >= USE_PARENT_TT_DEPTH_THRESHOLD)
            parent_transpose_table.reg(&search->board, hash_code, v, v);
    #endif
    */
    register_tt(search, hash_code, first_alpha, v, best_move, alpha, beta, alpha, beta);
    //cerr << "best move " << best_move << endl;
    return make_pair(v, best_move);
}

int nega_alpha_ordering_single_thread(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta, skipped, legal, searching);
    if (!is_end_search && depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped, searching);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        if (depth >= USE_PARENT_TT_DEPTH_THRESHOLD){
            parent_transpose_table.get(&search->board, hash_code, &l, &u);
            if (u == l)
                return u;
            if (beta <= l)
                return l;
            if (u <= alpha)
                return u;
            alpha = max(alpha, l);
            beta = min(beta, u);
        }
    #endif
    int first_alpha = alpha;
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_ordering_single_thread(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED, is_end_search, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, is_end_search, &v, searching))
                return v;
        }
    #endif
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_alpha_ordering_single_thread(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, is_end_search, searching);
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, is_end_search, searching);
        for (const Flip &flip: move_list){
            eval_move(search, &flip);
            search->board.move(&flip);
                g = -nega_alpha_ordering_single_thread(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
            search->board.undo(&flip);
            eval_undo(search, &flip);
            alpha = max(alpha, g);
            if (v < g){
                v = g;
                best_move = flip.pos;
            }
            if (beta <= alpha)
                break;
        }
    }
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}

int nega_scout_single_thread(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (is_end_search && depth <= MID_TO_END_DEPTH)
        return nega_alpha_end(search, alpha, beta, skipped, legal, searching);
    if (!is_end_search && depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped, searching);
    ++(search->n_nodes);
    #if USE_MID_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #elif USE_END_SC
        if (is_end_search){
            int stab_res = stability_cut(search, &alpha, &beta);
            if (stab_res != SCORE_UNDEFINED)
                return stab_res;
        }
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_MID_TC
        int l, u;
        if (depth >= USE_PARENT_TT_DEPTH_THRESHOLD){
            parent_transpose_table.get(&search->board, hash_code, &l, &u);
            if (u == l)
                return u;
            if (beta <= l)
                return l;
            if (u <= alpha)
                return u;
            alpha = max(alpha, l);
            beta = min(beta, u);
        }
    #endif
    int first_alpha = alpha;
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_scout_single_thread(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED, is_end_search, searching);
        search->board.pass();
        search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, is_end_search, &v, searching))
                return v;
        }
    #endif
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            eval_move(search, &flip_best);
            search->board.move(&flip_best);
                g = -nega_scout_single_thread(search, -beta, -alpha, depth - 1, false, LEGAL_UNDEFINED, is_end_search, searching);
            search->board.undo(&flip_best);
            eval_undo(search, &flip_best);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering(search, move_list, depth, alpha, beta, is_end_search, searching);
        //bool searching = true;
        for (const Flip &flip: move_list){
            eval_move(search, &flip);
            search->board.move(&flip);
                if (v == -INF)
                    g = -nega_scout_single_thread(search, -beta, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                else{
                    g = -nega_alpha_ordering_single_thread(search, -alpha - 1, -alpha, depth - 1, false, flip.n_legal, is_end_search, searching);
                    if (alpha < g && g < beta)
                        g = -nega_scout_single_thread(search, -beta, -g, depth - 1, false, flip.n_legal, is_end_search, searching);
                }
            search->board.undo(&flip);
            eval_undo(search, &flip);
            alpha = max(alpha, g);
            if (v < g){
                v = g;
                best_move = flip.pos;
            }
            if (beta <= alpha)
                break;
        }
    }
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}