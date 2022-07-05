#pragma once
#include <iostream>
#include <vector>
#include <functional>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "search.hpp"
#include "move_ordering.hpp"
#include "probcut.hpp"
#include "transpose_table.hpp"
#if USE_MULTI_THREAD
    #include "thread_pool.hpp"
    #include "ybwc.hpp"
#endif
#include "util.hpp"
#include "stability.hpp"

using namespace std;
/*
int nega_alpha_end_nomemo(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (depth <= MID_FAST_DEPTH)
        return nega_alpha(search, alpha, beta, depth, skipped);
    //if (depth == 1)
    //    return nega_alpha_eval1(search, alpha, beta, skipped);
    ++(search->n_nodes);
    #if USE_END_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_end_nomemo(search, -beta, -alpha, depth, true, LEGAL_UNDEFINED);
        search->board.pass();
        return v;
    }
    #if USE_MID_MPC
        if (search->use_mpc){
            if (mpc(search, alpha, beta, depth, legal, false, &v))
                return v;
        }
    #endif
    const int canput = pop_count_ull(legal);
    vector<Flip> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
        calc_flip(&move_list[idx++], &search->board, cell);
    move_ordering(search, move_list, depth, alpha, beta, false);
    for (const Flip &flip: move_list){
        eval_move(search, &flip);
        search->board.move(&flip);
            g = -nega_alpha_end_nomemo(search, -beta, -alpha, depth - 1, false, flip.n_legal);
        search->board.undo(&flip);
        eval_undo(search, &flip);
        alpha = max(alpha, g);
        v = max(v, g);
        if (beta <= alpha)
            break;
    }
    return v;
}
*/
/*
inline int last1(Search *search, int alpha, int beta, uint_fast8_t p0){
    ++search->n_nodes;
    int score = HW2 - 2 * search->board.count_opponent();
    int n_flip = count_last_flip(search->board.player, search->board.opponent, p0);
    if (n_flip == 0){
        ++search->n_nodes;
        n_flip = count_last_flip(search->board.opponent, search->board.player, p0);
        if (n_flip == 0){
            if (score < 1)
                score -= 2;
        } else
            score -= 2 * n_flip + 2;
    } else
        score += 2 * n_flip;
    return score_to_value(score);
}
*/

/*
inline int last1_without_pass(Search *search, int score, const int alpha, uint_fast8_t p0){
    ++search->n_nodes;
    int n_flip = 0;
    if (score <= 0){
        score -= 2;
        if (score >= alpha){
            if (bit_around[p0] & search->board.player){
                n_flip = count_last_flip(search->board.opponent, search->board.player, p0);
                score -= 2 * n_flip;
            }
        }
    } else{
        if (score >= alpha){
            if (bit_around[p0] & search->board.player){
                n_flip = count_last_flip(search->board.opponent, search->board.player, p0);
                if (n_flip)
                    score -= 2 * n_flip + 2;
            }
        }
    }
    return score;
}

inline int last1(Search *search, int alpha, int beta, uint_fast8_t p0){
    ++search->n_nodes;
    int score = HW2 - 2 * search->board.count_opponent();
    int n_flip = 0;
    if (bit_around[p0] & search->board.opponent){
        n_flip = count_last_flip(search->board.player, search->board.opponent, p0);
        if (n_flip == 0)
            score = last1_without_pass(search, score, alpha, p0);
        else
            score += 2 * n_flip;
    } else
        score = last1_without_pass(search, score, alpha, p0);
    return score;
}
*/

inline int last1(Search *search, int alpha, int beta, uint_fast8_t p0){
    ++search->n_nodes;
    int score = HW2 - 2 * search->board.count_opponent();
    int n_flip;
    n_flip = count_last_flip(search->board.player, search->board.opponent, p0);
    if (n_flip == 0){
        ++search->n_nodes;
        if (score <= 0){
            score -= 2;
            if (score >= alpha){
                n_flip = count_last_flip(search->board.opponent, search->board.player, p0);
                score -= 2 * n_flip;
            }
        } else{
            if (score >= alpha){
                n_flip = count_last_flip(search->board.opponent, search->board.player, p0);
                if (n_flip)
                    score -= 2 * n_flip + 2;
            }
        }
    } else
        score += 2 * n_flip;
    return score;
}

inline int last2(Search *search, int alpha, int beta, uint_fast8_t p0, uint_fast8_t p1, bool skipped){
    ++search->n_nodes;
    #if USE_END_PO & false
        uint_fast8_t p0_parity = (search->board.parity & cell_div4[p0]);
        uint_fast8_t p1_parity = (search->board.parity & cell_div4[p1]);
        if (!p0_parity && p1_parity)
            swap(p0, p1);
    #endif
    int v, g;
    Flip flip;
    if (bit_around[p0] & search->board.opponent){
        calc_flip(&flip, &search->board, p0);
        if (flip.flip){
            search->board.move(&flip);
                g = -last1(search, -beta, -alpha, p1);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (beta <= alpha)
                return alpha;
            v = g;
        } else
            v = -INF;
    } else
        v = -INF;
    if (bit_around[p1] & search->board.opponent){
        calc_flip(&flip, &search->board, p1);
        if (flip.flip){
            search->board.move(&flip);
                g = -last1(search, -beta, -alpha, p0);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (beta <= alpha)
                return alpha;
            v = max(v, g);
        }
    }
    if (v == -INF){
        if (skipped)
            v = end_evaluate(&search->board);
        else{
            search->board.pass();
                v = -last2(search, -beta, -alpha, p0, p1, true);
            search->board.pass();
        }
    }
    return v;
}

inline int last3(Search *search, int alpha, int beta, uint_fast8_t p0, uint_fast8_t p1, uint_fast8_t p2, bool skipped){
    ++search->n_nodes;
    #if USE_END_PO
        if (!skipped){
            const bool p0_parity = (search->board.parity & cell_div4[p0]) > 0;
            const bool p1_parity = (search->board.parity & cell_div4[p1]) > 0;
            const bool p2_parity = (search->board.parity & cell_div4[p2]) > 0;
            #if LAST_PO_OPTIMISE
                if (!p0_parity && p2_parity){
                    swap(p0, p2);
                } else if (!p0_parity && p1_parity && !p2_parity){
                    swap(p0, p1);
                } else if (p0_parity && !p1_parity && p2_parity){
                    swap(p1, p2);
                }
            #else
                if (!p0_parity && p1_parity && p2_parity){
                    swap(p0, p2);
                } else if (!p0_parity && !p1_parity && p2_parity){
                    swap(p0, p2);
                } else if (!p0_parity && p1_parity && !p2_parity){
                    swap(p0, p1);
                } else if (p0_parity && !p1_parity && p2_parity){
                    swap(p1, p2);
                }
            #endif
        }
    #endif
    //uint64_t legal = search->board.get_legal();
    int v = -INF, g;
    Flip flip;
    if (bit_around[p0] & search->board.opponent){
        calc_flip(&flip, &search->board, p0);
        if (flip.flip){
            search->board.move(&flip);
                g = -last2(search, -beta, -alpha, p1, p2, false);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (beta <= alpha)
                return alpha;
            v = g;
        }
    }
    if (bit_around[p1] & search->board.opponent){
        calc_flip(&flip, &search->board, p1);
        if (flip.flip){
            search->board.move(&flip);
                g = -last2(search, -beta, -alpha, p0, p2, false);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (beta <= alpha)
                return alpha;
            v = max(v, g);
        }
    }
    if (bit_around[p2] & search->board.opponent){
        calc_flip(&flip, &search->board, p2);
        if (flip.flip){
            search->board.move(&flip);
                g = -last2(search, -beta, -alpha, p0, p1, false);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            if (beta <= alpha)
                return alpha;
            v = max(v, g);
        }
    }
    if (v == -INF){
        if (skipped)
            v = end_evaluate(&search->board);
        else{
            search->board.pass();
                v = -last3(search, -beta, -alpha, p0, p1, p2, true);
            search->board.pass();
        }
        return v;
    }
    return v;
}

inline int last4(Search *search, int alpha, int beta, uint_fast8_t p0, uint_fast8_t p1, uint_fast8_t p2, uint_fast8_t p3, bool skipped){
    ++search->n_nodes;
    #if USE_END_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED){
            return stab_res;
        }
    #endif
    #if USE_END_PO
        if (!skipped){
            const bool p0_parity = (search->board.parity & cell_div4[p0]) > 0;
            const bool p1_parity = (search->board.parity & cell_div4[p1]) > 0;
            const bool p2_parity = (search->board.parity & cell_div4[p2]) > 0;
            const bool p3_parity = (search->board.parity & cell_div4[p3]) > 0;
            #if LAST_PO_OPTIMISE
                if (!p0_parity && p3_parity){
                    swap(p0, p3);
                    if (!p1_parity && p2_parity)
                        swap(p1, p2);
                } else if (!p0_parity && p2_parity && !p3_parity){
                    swap(p0, p2);
                } else if (!p0_parity && p1_parity && !p2_parity && !p3_parity){
                    swap(p0, p1);
                } else if (p0_parity && !p1_parity && p3_parity){
                    swap(p1, p3);
                } else if (p0_parity && !p1_parity && p2_parity && !p3_parity){
                    swap(p1, p2);
                } else if (p0_parity && p1_parity && !p2_parity && p3_parity){
                    swap(p2, p3);
                }
            #else
                if (!p0_parity && p1_parity && p2_parity && p3_parity){
                    swap(p0, p3);
                } else if (!p0_parity && !p1_parity && p2_parity && p3_parity){
                    swap(p0, p2);
                    swap(p1, p3);
                } else if (!p0_parity && p1_parity && !p2_parity && p3_parity){
                    swap(p0, p3);
                } else if (!p0_parity && p1_parity && p2_parity && !p3_parity){
                    swap(p0, p2);
                } else if (!p0_parity && !p1_parity && !p2_parity && p3_parity){
                    swap(p0, p3);
                } else if (!p0_parity && !p1_parity && p2_parity && !p3_parity){
                    swap(p0, p2);
                } else if (!p0_parity && p1_parity && !p2_parity && !p3_parity){
                    swap(p0, p1);
                } else if (p0_parity && !p1_parity && p2_parity && p3_parity){
                    swap(p1, p3);
                } else if (p0_parity && !p1_parity && !p2_parity && p3_parity){
                    swap(p1, p3);
                } else if (p0_parity && !p1_parity && p2_parity && !p3_parity){
                    swap(p1, p2);
                } else if (p0_parity && p1_parity && !p2_parity && p3_parity){
                    swap(p2, p3);
                }
            #endif
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int v = -INF, g;
    if (legal == 0ULL){
        if (skipped)
            v = end_evaluate(&search->board);
        else{
            search->board.pass();
                v = -last4(search, -beta, -alpha, p0, p1, p2, p3, true);
            search->board.pass();
        }
        return v;
    }
    Flip flip;
    if (1 & (legal >> p0)){
        calc_flip(&flip, &search->board, p0);
        search->board.move(&flip);
            g = -last3(search, -beta, -alpha, p1, p2, p3, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    if (1 & (legal >> p1)){
        calc_flip(&flip, &search->board, p1);
        search->board.move(&flip);
            g = -last3(search, -beta, -alpha, p0, p2, p3, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    if (1 & (legal >> p2)){
        calc_flip(&flip, &search->board, p2);
        search->board.move(&flip);
            g = -last3(search, -beta, -alpha, p0, p1, p3, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    if (1 & (legal >> p3)){
        calc_flip(&flip, &search->board, p3);
        search->board.move(&flip);
            g = -last3(search, -beta, -alpha, p0, p1, p2, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    return v;
}

inline void pick_vacant(Search *search, uint_fast8_t cells[]){
    int idx = 0;
    uint64_t empties = ~(search->board.player | search->board.opponent);
    //uint64_t empties_copy;
    //for (int i = 0; i < N_CELL_WEIGHT_MASK; ++i){
    //    empties_copy = empties & cell_weight_mask[i];
    //    for (uint_fast8_t cell = first_bit(&empties_copy); empties_copy; cell = next_bit(&empties_copy))
    //        cells[idx++] = cell;
    //}
    for (uint_fast8_t cell = first_bit(&empties); empties; cell = next_bit(&empties))
        cells[idx++] = cell;
}

int nega_alpha_end_fast(Search *search, int alpha, int beta, bool skipped, bool stab_cut, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (search->board.n == 60){
        //uint_fast8_t cells[4];
        //pick_vacant(search, cells);
        //return last4(search, alpha, beta, cells[0], cells[1], cells[2], cells[3], skipped);
        uint64_t empties = ~(search->board.player | search->board.opponent);
        uint_fast8_t p0, p1, p2, p3;
        p0 = first_bit(&empties);
        p1 = next_bit(&empties);
        p2 = next_bit(&empties);
        p3 = next_bit(&empties);
        return last4(search, alpha, beta, p0, p1, p2, p3, skipped);
    }
    ++search->n_nodes;
    #if USE_END_SC
        if (stab_cut){
            int stab_res = stability_cut(search, &alpha, &beta);
            if (stab_res != SCORE_UNDEFINED){
                return stab_res;
            }
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_end_fast(search, -beta, -alpha, true, false, searching);
        search->board.pass();
        return v;
    }
    Flip flip;
    #if USE_END_PO
        int i;
        uint64_t legal_copy;
        uint_fast8_t cell;
        if (0 < search->board.parity && search->board.parity < 15){
            uint64_t legal_mask = 0ULL;
            if (search->board.parity & 1)
                legal_mask |= 0x000000000F0F0F0FULL;
            if (search->board.parity & 2)
                legal_mask |= 0x00000000F0F0F0F0ULL;
            if (search->board.parity & 4)
                legal_mask |= 0x0F0F0F0F00000000ULL;
            if (search->board.parity & 8)
                legal_mask |= 0xF0F0F0F000000000ULL;
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & legal_mask & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false, true, searching);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (beta <= alpha)
                            return alpha;
                        v = max(v, g);
                    }
                }
            }
            legal_mask = ~legal_mask;
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & legal_mask & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false, true, searching);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (beta <= alpha)
                            return alpha;
                        v = max(v, g);
                    }
                }
            }
        } else{
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false, true, searching);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (beta <= alpha)
                            return alpha;
                        v = max(v, g);
                    }
                }
            }
        }
    #else
        uint64_t legal_copy;
        uint_fast8_t cell;
        int i;
        for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
            legal_copy = legal & cell_weight_mask[i];
            if (legal_copy){
                for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                    calc_flip(&flip, &search->board, cell);
                    search->board.move(&flip);
                        g = -nega_alpha_end_fast(search, -beta, -alpha, false, true, searching);
                    search->board.undo(&flip);
                    alpha = max(alpha, g);
                    if (beta <= alpha)
                        return alpha;
                    v = max(v, g);
                }
            }
        }
    #endif
    if (!(*searching))
        return SCORE_UNDEFINED;
    return v;
}

/* with tt
int nega_alpha_end_fast(Search *search, int alpha, int beta, bool skipped){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (search->board.n == 60){
        //uint_fast8_t cells[4];
        //pick_vacant(search, cells);
        //return last4(search, alpha, beta, cells[0], cells[1], cells[2], cells[3], skipped);
        uint64_t empties = ~(search->board.player | search->board.opponent);
        uint_fast8_t p0, p1, p2, p3;
        p0 = first_bit(&empties);
        p1 = next_bit(&empties);
        p2 = next_bit(&empties);
        p3 = next_bit(&empties);
        return last4(search, alpha, beta, p0, p1, p2, p3, skipped);
    }
    ++search->n_nodes;
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_END_TC
        int l, u;
        if (search->board.n <= HW2 - USE_TT_DEPTH_THRESHOLD){
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
    #if USE_END_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED){
            register_tt(search, hash_code, first_alpha, stab_res, TRANSPOSE_TABLE_UNDEFINED, l, u, alpha, beta);
            return stab_res;
        }
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_end_fast(search, -beta, -alpha, true);
        search->board.pass();
        return v;
    }
    Flip flip;
    int best_move = child_transpose_table.get(&search->board, hash_code);
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            calc_flip(&flip, &search->board, best_move);
            search->board.move(&flip);
                g = -nega_alpha_end_fast(search, -beta, -alpha, false);
            search->board.undo(&flip);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    #if USE_END_PO
        if (0 < search->board.parity && search->board.parity < 15){
            uint64_t legal_mask = 0ULL;
            if (search->board.parity & 1)
                legal_mask |= 0x000000000F0F0F0FULL;
            if (search->board.parity & 2)
                legal_mask |= 0x00000000F0F0F0F0ULL;
            if (search->board.parity & 4)
                legal_mask |= 0x0F0F0F0F00000000ULL;
            if (search->board.parity & 8)
                legal_mask |= 0xF0F0F0F000000000ULL;
            uint64_t legal_copy;
            uint_fast8_t cell;
            int i;
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & legal_mask & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = cell;
                            if (beta <= alpha){
                                register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
                                return alpha;
                            }
                        }
                    }
                }
            }
            legal_mask = ~legal_mask;
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & legal_mask & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = cell;
                            if (beta <= alpha){
                                register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
                                return alpha;
                            }
                        }
                    }
                }
            }
        } else{
            uint64_t legal_copy;
            uint_fast8_t cell;
            int i;
            for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                legal_copy = legal & cell_weight_mask[i];
                if (legal_copy){
                    for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                        calc_flip(&flip, &search->board, cell);
                        search->board.move(&flip);
                            g = -nega_alpha_end_fast(search, -beta, -alpha, false);
                        search->board.undo(&flip);
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = cell;
                            if (beta <= alpha){
                                register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
                                return alpha;
                            }
                        }
                    }
                }
            }
        }
    #else
        uint64_t legal_copy;
        uint_fast8_t cell;
        int i;
        for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
            legal_copy = legal & cell_weight_mask[i];
            if (legal_copy){
                for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                    calc_flip(&flip, &search->board, cell);
                    search->board.move(&flip);
                        g = -nega_alpha_end_fast(search, -beta, -alpha, false);
                    search->board.undo(&flip);
                    alpha = max(alpha, g);
                    if (v < g){
                        v = g;
                        best_move = cell;
                        if (beta <= alpha){
                            register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
                            return alpha;
                        }
                    }
                }
            }
        }
    #endif
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}
*/

/*
int nega_alpha_end_fast(Search *search, int alpha, int beta, bool skipped){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (search->board.n == 60){
        uint_fast8_t cells[4];
        pick_vacant(search, cells);
        return last4(search, alpha, beta, cells[0], cells[1], cells[2], cells[3], skipped);
    }
    ++search->n_nodes;
    #if USE_END_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint64_t legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        search->board.pass();
            v = -nega_alpha_end_fast(search, -beta, -alpha, true);
        search->board.pass();
        return v;
    }
    const int canput = pop_count_ull(legal);
    vector<Flip> move_list(canput);
    int idx = 0;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
        calc_flip(&move_list[idx++], &search->board, cell);
    move_ordering_fast_first(search, move_list);
    for (const Flip &flip: move_list){
        search->board.move(&flip);
            g = -nega_alpha_end_fast(search, -beta, -alpha, false);
        search->board.undo(&flip);
        alpha = max(alpha, g);
        if (beta <= alpha)
            return alpha;
        v = max(v, g);
    }
    return v;
}
*/
/*
int nega_alpha_end(Search *search, int alpha, int beta, bool skipped, uint64_t legal, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (search->board.n >= HW2 - END_FAST_DEPTH)
        return nega_alpha_end_fast(search, alpha, beta, skipped);
    ++search->n_nodes;
    #if USE_END_SC
        int stab_res = stability_cut(search, &alpha, &beta);
        if (stab_res != SCORE_UNDEFINED)
            return stab_res;
    #endif
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_END_TC
        int l, u;
        parent_transpose_table.get(&search->board, hash_code, &l, &u);
        if (u == l)
            return u;
        if (beta <= l)
            return l;
        if (u <= alpha)
            return u;
        alpha = max(alpha, l);
        beta = min(beta, u);
    #endif
    if (legal == LEGAL_UNDEFINED)
        legal = search->board.get_legal();
    int g, v = -INF;
    if (legal == 0ULL){
        if (skipped)
            return end_evaluate(&search->board);
        //search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_end(search, -beta, -alpha, true, LEGAL_UNDEFINED, searching);
        search->board.pass();
        //search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_END_MPC && false
        if (search->use_mpc){
            if (mpc(search, alpha, beta, HW2 - search->board.n, legal, false, &v))
                return v;
        }
    #endif
    int best_move = child_transpose_table.get(&search->board, hash_code);
    int f_best_move = best_move;
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            //eval_move(search, &flip);
            search->board.move(&flip_best);
                g = -nega_alpha_end(search, -beta, -alpha, false, LEGAL_UNDEFINED, searching);
            search->board.undo(&flip_best);
            //eval_undo(search, &flip);
            alpha = max(alpha, g);
            v = g;
            legal ^= 1ULL << best_move;
        }
    }
    if (alpha < beta && legal){
        const int canput = pop_count_ull(legal);
        vector<Flip> move_list(canput);
        int idx = 0;
        for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
            calc_flip(&move_list[idx++], &search->board, cell);
        move_ordering_end(search, move_list);
        for (const Flip &flip: move_list){
            //eval_move(search, &flip);
            search->board.move(&flip);
                g = -nega_alpha_end(search, -beta, -alpha, false, flip.n_legal, searching);
            search->board.undo(&flip);
            //eval_undo(search, &flip);
            alpha = max(alpha, g);
            if (v < g){
                v = g;
                best_move = flip.pos;
            }
            if (beta <= alpha)
                break;
        }
    }
    if (best_move != f_best_move)
        child_transpose_table.reg(&search->board, hash_code, best_move);
    #if USE_END_TC
        if (beta <= v && l < v)
            parent_transpose_table.reg(&search->board, hash_code, v, u);
        else if (v <= alpha && v < u)
            parent_transpose_table.reg(&search->board, hash_code, l, v);
        else if (alpha < v && v < beta)
            parent_transpose_table.reg(&search->board, hash_code, v, v);
    #endif
    return v;
}
*/

int nega_alpha_end(Search *search, int alpha, int beta, bool skipped, uint64_t legal, const bool *searching){
    if (!global_searching || !(*searching))
        return SCORE_UNDEFINED;
    if (search->board.n >= HW2 - END_FAST_DEPTH)
        return nega_alpha_end_fast(search, alpha, beta, skipped, false, searching);
    ++search->n_nodes;
    uint32_t hash_code = search->board.hash() & TRANSPOSE_TABLE_MASK;
    #if USE_END_TC
        int l = -SCORE_MAX, u = SCORE_MAX;
        if (search->board.n <= HW2 - USE_TT_DEPTH_THRESHOLD){
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
        //search->eval_feature_reversed ^= 1;
        search->board.pass();
            v = -nega_alpha_end(search, -beta, -alpha, true, LEGAL_UNDEFINED, searching);
        search->board.pass();
        //search->eval_feature_reversed ^= 1;
        return v;
    }
    #if USE_END_MPC && false
        if (search->use_mpc){
            if (mpc(search, alpha, beta, HW2 - search->board.n, legal, false, &v, searching))
                return v;
        }
    #endif
    int best_move = TRANSPOSE_TABLE_UNDEFINED;
    if (search->board.n <= HW2 - USE_TT_DEPTH_THRESHOLD)
        best_move = child_transpose_table.get(&search->board, hash_code);
    int stab_res;
    if (best_move != TRANSPOSE_TABLE_UNDEFINED){
        if (1 & (legal >> best_move)){
            Flip flip_best;
            calc_flip(&flip_best, &search->board, best_move);
            //eval_move(search, &flip);
            search->board.move(&flip_best);
                #if USE_END_SC
                    stab_res = stability_cut_move(search, &flip_best, &alpha, &beta);
                    if (stab_res != SCORE_UNDEFINED){
                        search->board.undo(&flip_best);
                        register_tt(search, hash_code, first_alpha, stab_res, best_move, l, u, alpha, beta);
                        return stab_res;
                    }
                #endif
                g = -nega_alpha_end(search, -beta, -alpha, false, LEGAL_UNDEFINED, searching);
            search->board.undo(&flip_best);
            if (*searching){
                //eval_undo(search, &flip);
                alpha = max(alpha, g);
                v = g;
                legal ^= 1ULL << best_move;
            } else
                return SCORE_UNDEFINED;
        } else
            best_move = TRANSPOSE_TABLE_UNDEFINED;
    }
    if (alpha < beta && legal){
        if (best_move == TRANSPOSE_TABLE_UNDEFINED){
            const int canput = pop_count_ull(legal);
            vector<Flip> move_list(canput);
            int idx = 0;
            for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal))
                calc_flip(&move_list[idx++], &search->board, cell);
            move_evaluate_fast_first(search, move_list);
            #if USE_MULTI_THREAD && false
                int pv_idx = 0, split_count = 0;
                if (best_move != TRANSPOSE_TABLE_UNDEFINED)
                    pv_idx = 1;
                vector<future<pair<int, uint64_t>>> parallel_tasks;
                bool n_searching = true;
                int depth = HW2 - pop_count_ull(search->board.player | search->board.opponent);
                for (const Flip &flip: move_list){
                    if (!(*searching))
                        break;
                    search->board.move(&flip);
                        if (ybwc_split_without_move_end(search, &flip, -beta, -alpha, depth - 1, flip.n_legal, &n_searching, pv_idx++, canput, split_count, parallel_tasks, move_list[0].value)){
                            ++split_count;
                        } else{
                            g = -nega_alpha_end(search, -beta, -alpha, false, flip.n_legal, searching);
                            if (*searching){
                                alpha = max(alpha, g);
                                if (v < g){
                                    v = g;
                                    best_move = flip.pos;
                                }
                                if (beta <= alpha){
                                    search->board.undo(&flip);
                                    break;
                                }
                            }
                        }
                    search->board.undo(&flip);
                }
                if (split_count){
                    if (beta <= alpha || !(*searching)){
                        n_searching = false;
                        ybwc_wait_all(search, parallel_tasks);
                    } else{
                        g = ybwc_wait_all(search, parallel_tasks);
                        alpha = max(alpha, g);
                        v = max(v, g);
                    }
                }
            #else
                const int move_ordering_threshold = MOVE_ORDERING_THRESHOLD - (int)(best_move != TRANSPOSE_TABLE_UNDEFINED);
                for (int move_idx = 0; move_idx < canput; ++move_idx){
                    if (move_idx < move_ordering_threshold)
                        swap_next_best_move(move_list, move_idx, canput);
                    search->board.move(&move_list[move_idx]);
                        #if USE_END_SC
                            stab_res = stability_cut_move(search, &move_list[move_idx], &alpha, &beta);
                            if (stab_res != SCORE_UNDEFINED){
                                search->board.undo(&move_list[move_idx]);
                                register_tt(search, hash_code, first_alpha, stab_res, move_list[move_idx].pos, l, u, alpha, beta);
                                return stab_res;
                            }
                        #endif
                        g = -nega_alpha_end(search, -beta, -alpha, false, move_list[move_idx].n_legal, searching);
                    search->board.undo(&move_list[move_idx]);
                    if (*searching){
                        alpha = max(alpha, g);
                        if (v < g){
                            v = g;
                            best_move = move_list[move_idx].pos;
                            if (beta <= alpha){
                                register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
                                return alpha;
                            }
                        }
                    } else
                        return SCORE_UNDEFINED;
                }
            #endif
        } else{
            Flip flip;
            if (0 < search->board.parity && search->board.parity < 15){
                uint64_t legal_mask = 0ULL;
                if (search->board.parity & 1)
                    legal_mask |= 0x000000000F0F0F0FULL;
                if (search->board.parity & 2)
                    legal_mask |= 0x00000000F0F0F0F0ULL;
                if (search->board.parity & 4)
                    legal_mask |= 0x0F0F0F0F00000000ULL;
                if (search->board.parity & 8)
                    legal_mask |= 0xF0F0F0F000000000ULL;
                uint64_t legal_copy;
                uint_fast8_t cell;
                int i;
                for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                    legal_copy = legal & legal_mask & cell_weight_mask[i];
                    if (legal_copy){
                        for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                            calc_flip(&flip, &search->board, cell);
                            search->board.move(&flip);
                                #if USE_END_SC
                                    stab_res = stability_cut_move(search, &flip, &alpha, &beta);
                                    if (stab_res != SCORE_UNDEFINED){
                                        search->board.undo(&flip);
                                        register_tt(search, hash_code, first_alpha, stab_res, flip.pos, l, u, alpha, beta);
                                        return stab_res;
                                    }
                                #endif
                                g = -nega_alpha_end(search, -beta, -alpha, false, LEGAL_UNDEFINED, searching);
                            search->board.undo(&flip);
                            if (*searching){
                                alpha = max(alpha, g);
                                v = max(v, g);
                                if (beta <= alpha){
                                    register_tt(search, hash_code, first_alpha, v, cell, l, u, alpha, beta);
                                    return alpha;
                                }
                            } else
                                return SCORE_UNDEFINED;
                        }
                    }
                }
                legal_mask = ~legal_mask;
                for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                    legal_copy = legal & legal_mask & cell_weight_mask[i];
                    if (legal_copy){
                        for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                            calc_flip(&flip, &search->board, cell);
                            search->board.move(&flip);
                                #if USE_END_SC
                                    stab_res = stability_cut_move(search, &flip, &alpha, &beta);
                                    if (stab_res != SCORE_UNDEFINED){
                                        search->board.undo(&flip);
                                        register_tt(search, hash_code, first_alpha, stab_res, flip.pos, l, u, alpha, beta);
                                        return stab_res;
                                    }
                                #endif
                                g = -nega_alpha_end(search, -beta, -alpha, false, LEGAL_UNDEFINED, searching);
                            search->board.undo(&flip);
                            if (*searching){
                                alpha = max(alpha, g);
                                v = max(v, g);
                                if (beta <= alpha){
                                    register_tt(search, hash_code, first_alpha, v, cell, l, u, alpha, beta);
                                    return alpha;
                                }
                            } else
                                return SCORE_UNDEFINED;
                        }
                    }
                }
            } else{
                uint64_t legal_copy;
                uint_fast8_t cell;
                int i;
                for (i = 0; i < N_CELL_WEIGHT_MASK; ++i){
                    legal_copy = legal & cell_weight_mask[i];
                    if (legal_copy){
                        for (cell = first_bit(&legal_copy); legal_copy; cell = next_bit(&legal_copy)){
                            calc_flip(&flip, &search->board, cell);
                            search->board.move(&flip);
                                #if USE_END_SC
                                    stab_res = stability_cut_move(search, &flip, &alpha, &beta);
                                    if (stab_res != SCORE_UNDEFINED){
                                        search->board.undo(&flip);
                                        register_tt(search, hash_code, first_alpha, stab_res, flip.pos, l, u, alpha, beta);
                                        return stab_res;
                                    }
                                #endif
                                g = -nega_alpha_end(search, -beta, -alpha, false, LEGAL_UNDEFINED, searching);
                            search->board.undo(&flip);
                            if (*searching){
                                alpha = max(alpha, g);
                                v = max(v, g);
                                if (beta <= alpha){
                                    register_tt(search, hash_code, first_alpha, v, cell, l, u, alpha, beta);
                                    return alpha;
                                }
                            } else
                                return SCORE_UNDEFINED;
                        }
                    }
                }
            }
        }
    }
    register_tt(search, hash_code, first_alpha, v, best_move, l, u, alpha, beta);
    return v;
}
