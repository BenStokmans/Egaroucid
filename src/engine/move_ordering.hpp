/*
    Egaroucid Project

    @file move_ordering.hpp
        Move ordering for each search algorithm
    @date 2021-2022
    @author Takuto Yamana (a.k.a. Nyanyan)
    @license GPL-3.0 license
*/

#pragma once
#include <iostream>
#include <vector>
#include "common.hpp"
#include "board.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "stability.hpp"

/*
    @brief if wipeout found, it must be searched first.
*/
#define W_WIPEOUT INF

/*
    @brief constants for move ordering
*/
#define MOVE_ORDERING_VALUE_OFFSET_ALPHA 10
#define MOVE_ORDERING_VALUE_OFFSET_BETA 10
#define MOVE_ORDERING_NWS_VALUE_OFFSET_ALPHA 10
#define MOVE_ORDERING_NWS_VALUE_OFFSET_BETA 3
#define MOVE_ORDERING_MPCT 0.7
#define W_END_MOBILITY 16
#define W_END_PARITY 8

#if USE_AUTO_OPTIMIZED_MOVE_ORDERING_MID
    #ifndef N_MOVE_ORDERING_PHASES
        #define N_MOVE_ORDERING_PHASES 6
    #endif
    #define N_MOVE_ORDERING_WEIGHT 5
    constexpr int move_ordering_weights[N_MOVE_ORDERING_PHASES][N_MOVE_ORDERING_WEIGHT] = {
        {-27, -37, 25, -57, -3},
        {-11, -8, 19, -132, -16},
        {-24, -16, 6, -110, -2}, 
        {-24, -18, 15, -93, -28},
        {-27, -7, 32, -77, -46},
        {-90, -40, 31, -82, -75}
    };
#else
    #define W_VALUE_TT 6
    #define W_VALUE_DEEP 12
    #define W_VALUE 10
    #define W_VALUE_SHALLOW 8
    #define W_MOBILITY 8
    #define W_PLAYER_POTENTIAL_MOBILITY 8
    #define W_OPPONENT_POTENTIAL_MOBILITY 10
    //#define W_OPENNESS 1
#endif

#if USE_AUTO_OPTIMIZED_MOVE_ORDERING_NWS
    #ifndef N_MOVE_ORDERING_PHASES
        #define N_MOVE_ORDERING_PHASES 6
    #endif
    #define N_MOVE_ORDERING_NWS_WEIGHT 5
    constexpr int move_ordering_nws_weights[N_MOVE_ORDERING_PHASES][N_MOVE_ORDERING_NWS_WEIGHT] = {
        {-14, -12, 8, -12, -14},
        {-14, -12, 8, -12, -14},
        {-14, -12, 8, -12, -14},
        {-14, -10, 6, -12, -14},
        {-14, -8, 4, -10, -12},
        {-14, -6, 2, -10, -12}
    };
#else
    #define W_NWS_MOBILITY 14
    //#define W_NWS_N_FLIP 2
    #define W_NWS_VALUE_DEEP 16
    #define W_NWS_VALUE 14
    #define W_NWS_VALUE_SHALLOW 12
    //#define W_NWS_N_NODES 2
    #define W_NWS_PLAYER_POTENTIAL_MOBILITY 8
    #define W_NWS_OPPONENT_POTENTIAL_MOBILITY 12
#endif

/*
    @brief Flip structure with more information

    @param flip                 flip information
    @param value                the move ordering value
    @param n_legal              next legal moves as a bitboard for reusing
*/
struct Flip_value{
    Flip flip;
    int value;
    uint64_t n_legal;
    Flip_value(){
        n_legal = LEGAL_UNDEFINED;
    }
};

#if USE_AUTO_OPTIMIZED_MOVE_ORDERING || USE_AUTO_OPTIMIZED_MOVE_ORDERING_NWS
    #define N_MOVE_ORDERING_PHASE_DISCS 10
    /*
        @brief Get a phase for move ordering

        @param n_discs              number of discs on the board
        @return move ordering phase
    */
    inline int get_move_ordering_phase(int n_discs){
        return (n_discs - 4) / N_MOVE_ORDERING_PHASE_DISCS;
    }
#endif

int nega_alpha_eval1(Search *search, int alpha, int beta, bool skipped, const bool *searching);
#if MID_FAST_DEPTH > 1
    int nega_alpha(Search *search, int alpha, int beta, int depth, bool skipped, const bool *searching);
#endif
int nega_scout(Search *search, int alpha, int beta, int depth, bool skipped, uint64_t legal, bool is_end_search, const bool *searching);

/*
    @brief Calculate openness

    Not used for now

    @param board                board
    @param flip                 flip information
    @return openness
*/
/*
inline int calc_openness(const Board *board, const Flip *flip){
    uint64_t f = flip->flip;
    uint64_t around = 0ULL;
    for (uint_fast8_t cell = first_bit(&f); f; cell = next_bit(&f))
        around |= bit_around[cell];
    around &= ~flip->flip;
    return pop_count_ull(~(board->player | board->opponent | (1ULL << flip->pos)) & around);
}
*/

/*
    @brief Get number of corner mobility

    Optimized for corner mobility

    @param legal                legal moves as a bitboard
    @return number of legal moves on corners
*/
inline int get_corner_mobility(uint64_t legal){
    int res = (int)((legal & 0b10000001ULL) + ((legal >> 56) & 0b10000001ULL));
    return (res & 0b11) + (res >> 7);
}

/*
    @brief Get a weighted mobility

    @param legal                legal moves as a bitboard
    @return weighted mobility
*/
inline int get_weighted_n_moves(uint64_t legal){
    return pop_count_ull(legal) * 2 + get_corner_mobility(legal);
}

/*
    @brief Get potential mobility

    Same idea as surround in evaluation function

    @param opponent             a bitboard representing opponent
    @param empties              a bitboard representing empty squares
    @return potential mobility
*/
#if USE_SIMD
    inline int get_potential_mobility(uint64_t opponent, uint64_t empties){
        const u64_4 shift(1, HW, HW_M1, HW_P1);
        const u64_4 mask(0x7E7E7E7E7E7E7E7EULL, 0x00FFFFFFFFFFFF00ULL, 0x007E7E7E7E7E7E00ULL, 0x007E7E7E7E7E7E00ULL);
        u64_4 op(opponent);
        op = op & mask;
        return pop_count_ull(empties & all_or((op << shift) | (op >> shift)));
    }
#else
    inline int get_potential_mobility(uint64_t opponent, uint64_t empties){
        uint64_t hmask = opponent & 0x7E7E7E7E7E7E7E7EULL;
        uint64_t vmask = opponent & 0x00FFFFFFFFFFFF00ULL;
        uint64_t hvmask = opponent & 0x007E7E7E7E7E7E00ULL;
        uint64_t res = 
            (hmask << 1) | (hmask >> 1) | 
            (vmask << HW) | (vmask >> HW) | 
            (hvmask << HW_M1) | (hvmask >> HW_M1) | 
            (hvmask << HW_P1) | (hvmask >> HW_P1);
        return pop_count_ull(empties & res);
    }
#endif

#if USE_AUTO_OPTIMIZED_MOVE_ORDERING_MID
    /*
        @brief Evaluate a move in midgame

        @param search               search information
        @param flip_value           flip with value
        @param alpha                alpha value to search
        @param beta                 beta value to search
        @param depth                depth to search
        @param searching            flag for terminating this search
        @param phase                phase for move ordering
        @return true if wipeout found else false
    */
    inline bool move_evaluate(Search *search, Flip_value *flip_value, int alpha, int beta, int depth, const bool *searching, const int phase){
        if (flip_value->flip.flip == search->board.opponent){
            flip_value->value = W_WIPEOUT;
            return true;
        }
        eval_move(search, &flip_value->flip);
        search->move(&flip_value->flip);
            flip_value->n_legal = calc_legal(search->board.player, search->board.opponent);
            flip_value->value = get_weighted_n_moves(flip_value->n_legal) * move_ordering_weights[phase][0];
            //flip_value->value += get_weighted_n_moves(calc_legal(search->board.opponent, search->board.player)) * move_ordering_weights[phase][1];
            uint64_t empties = ~(search->board.player | search->board.opponent);
            flip_value->value += get_potential_mobility(search->board.opponent, empties) * move_ordering_weights[phase][1];
            flip_value->value += get_potential_mobility(search->board.player, empties) * move_ordering_weights[phase][2];
            switch (depth){
                case 0:
                    flip_value->value += mid_evaluate_diff(search) * move_ordering_weights[phase][3];
                    break;
                case 1:
                    flip_value->value += nega_alpha_eval1(search, alpha, beta, false, searching) * (move_ordering_weights[phase][3] + move_ordering_weights[phase][4]);
                    break;
                default:
                    #if MID_FAST_DEPTH > 1
                        if (depth <= MID_FAST_DEPTH)
                            flip_value->value += nega_alpha(search, alpha, beta, depth, false, searching) * (move_ordering_weights[phase][3] + move_ordering_weights[phase][4] * depth);
                        else{
                            bool use_mpc = search->use_mpc;
                            double mpct = search->mpct;
                            search->use_mpc = true;
                            search->mpct = MOVE_ORDERING_MPCT;
                                flip_value->value += nega_scout(search, alpha, beta, depth, false, flip_value->n_legal, false, searching) * (move_ordering_weights[phase][3] + move_ordering_weights[phase][4] * depth);
                            search->use_mpc = use_mpc;
                            search->mpct = mpct;
                        }
                    #else
                        bool use_mpc = search->use_mpc;
                        double mpct = search->mpct;
                        search->use_mpc = true;
                        search->mpct = MOVE_ORDERING_MPCT;
                            flip_value->value += nega_scout(search, alpha, beta, depth, false, flip_value->n_legal, false, searching) * (move_ordering_weights[phase][3] + move_ordering_weights[phase][4] * depth);
                        search->use_mpc = use_mpc;
                        search->mpct = mpct;
                    #endif
                    break;
            }
        search->undo(&flip_value->flip);
        eval_undo(search, &flip_value->flip);
        return false;
    }
#else
    /*
        @brief Evaluate a move in midgame

        @param search               search information
        @param flip_value           flip with value
        @param alpha                alpha value to search
        @param beta                 beta value to search
        @param depth                depth to search
        @param searching            flag for terminating this search
        @return true if wipeout found else false
    */
    inline bool move_evaluate(Search *search, Flip_value *flip_value, int alpha, int beta, int depth, const bool *searching){
        if (flip_value->flip.flip == search->board.opponent){
            flip_value->value = W_WIPEOUT;
            return true;
        }
        flip_value->value = cell_weight[flip_value->flip.pos];
        //flip_value->value -= (calc_openness(&search->board, &flip_value->flip) >> 1) * W_OPENNESS;
        eval_move(search, &flip_value->flip);
        search->move(&flip_value->flip);
            flip_value->n_legal = search->board.get_legal();
            flip_value->value -= get_weighted_n_moves(flip_value->n_legal) * W_MOBILITY;
            uint64_t empties = ~(search->board.player | search->board.opponent);
            flip_value->value -= get_potential_mobility(search->board.opponent, empties) * W_OPPONENT_POTENTIAL_MOBILITY;
            flip_value->value += get_potential_mobility(search->board.player, empties) * W_PLAYER_POTENTIAL_MOBILITY;
            switch (depth){
                case 0:
                    flip_value->value += -mid_evaluate_diff(search) * W_VALUE_SHALLOW;
                    break;
                case 1:
                    flip_value->value += -nega_alpha_eval1(search, alpha, beta, false, searching) * W_VALUE;
                    break;
                default:
                    #if MID_FAST_DEPTH > 1
                        if (depth <= MID_FAST_DEPTH)
                            flip_value->value += -nega_alpha(search, alpha, beta, depth, false, searching) * (W_VALUE_DEEP + (depth - 1) * 2);
                        else{
                            bool use_mpc = search->use_mpc;
                            double mpct = search->mpct;
                            search->use_mpc = true;
                            search->mpct = MOVE_ORDERING_MPCT;
                            flip_value->value += -nega_scout(search, alpha, beta, depth, false, flip_value->n_legal, false, searching) * (W_VALUE_DEEP + (depth - 1) * 2);
                            search->use_mpc = use_mpc;
                            search->mpct = mpct;
                        }
                    #else
                        bool use_mpc = search->use_mpc;
                        double mpct = search->mpct;
                        search->use_mpc = true;
                        search->mpct = MOVE_ORDERING_MPCT;
                        flip_value->value += -nega_scout(search, alpha, beta, depth, false, flip_value->n_legal, false, searching) * (W_VALUE_DEEP + (depth - 1) * 2);
                        search->use_mpc = use_mpc;
                        search->mpct = mpct;
                    #endif
                    break;
            }
        search->undo(&flip_value->flip);
        eval_undo(search, &flip_value->flip);
        return false;
    }
#endif

/*
    @brief Evaluate a move in endgame

    @param search               search information
    @param flip_value           flip with value
    @return true if wipeout found else false
*/
inline bool move_evaluate_end(Search *search, Flip_value *flip_value){
    if (flip_value->flip.flip == search->board.opponent){
        flip_value->value = W_WIPEOUT;
        return true;
    }
    flip_value->value = cell_weight[flip_value->flip.pos];
    if (search->parity & cell_div4[flip_value->flip.pos])
        flip_value->value += W_END_PARITY;
    search->move(&flip_value->flip);
        flip_value->n_legal = search->board.get_legal();
        flip_value->value -= pop_count_ull(flip_value->n_legal) * W_END_MOBILITY;
    search->undo(&flip_value->flip);
    return false;
}

#if USE_AUTO_OPTIMIZED_MOVE_ORDERING_NWS
    /*
        @brief Evaluate a move in midgame for NWS

        @param search               search information
        @param flip_value           flip with value
        @param alpha                alpha value to search
        @param beta                 beta value to search
        @param depth                depth to search
        @param searching            flag for terminating this search
        @param phase                phase for move ordering
        @return true if wipeout found else false
    */
    inline bool move_evaluate_nws(Search *search, Flip_value *flip_value, int alpha, int beta, int depth, const bool *searching, const int phase){
        if (flip_value->flip.flip == search->board.opponent){
            flip_value->value = W_WIPEOUT;
            return true;
        }
        flip_value->value = cell_weight[flip_value->flip.pos];
        eval_move(search, &flip_value->flip);
        search->move(&flip_value->flip);
            flip_value->n_legal = search->board.get_legal();
            flip_value->value += get_weighted_n_moves(flip_value->n_legal) * move_ordering_nws_weights[phase][0];
            uint64_t empties = ~(search->board.player | search->board.opponent);
            flip_value->value += get_potential_mobility(search->board.opponent, empties) * move_ordering_nws_weights[phase][1];
            flip_value->value += get_potential_mobility(search->board.player, empties) * move_ordering_nws_weights[phase][2];
            if (depth == 0)
                flip_value->value += mid_evaluate_diff(search) * move_ordering_nws_weights[phase][3];
            else
                flip_value->value += nega_alpha_eval1(search, alpha, beta, false, searching) * move_ordering_nws_weights[phase][4];
        search->undo(&flip_value->flip);
        eval_undo(search, &flip_value->flip);
        return false;
    }
#else
    /*
        @brief Evaluate a move in midgame for NWS

        @param search               search information
        @param flip_value           flip with value
        @param alpha                alpha value to search
        @param beta                 beta value to search
        @param depth                depth to search
        @param searching            flag for terminating this search
        @return true if wipeout found else false
    */
    inline bool move_evaluate_nws(Search *search, Flip_value *flip_value, int alpha, int beta, int depth, const bool *searching){
        if (flip_value->flip.flip == search->board.opponent){
            flip_value->value = W_WIPEOUT;
            return true;
        }
        flip_value->value = cell_weight[flip_value->flip.pos];
        //flip_value->value -= pop_count_ull(flip_value->flip.flip) * W_NWS_N_FLIP;
        eval_move(search, &flip_value->flip);
        search->move(&flip_value->flip);
            uint64_t empties = ~(search->board.player | search->board.opponent);
            flip_value->value -= get_potential_mobility(search->board.opponent, empties) * W_NWS_OPPONENT_POTENTIAL_MOBILITY;
            flip_value->value += get_potential_mobility(search->board.player, empties) * W_NWS_PLAYER_POTENTIAL_MOBILITY;
            flip_value->n_legal = search->board.get_legal();
            //flip_value->value -= pop_count_ull(flip_value->n_legal) * W_NWS_MOBILITY;
            flip_value->value -= get_weighted_n_moves(flip_value->n_legal) * W_NWS_MOBILITY;
            //int64_t bef_n_nodes = search->n_nodes;
            if (depth == 0)
                flip_value->value += -mid_evaluate_diff(search) * W_NWS_VALUE_SHALLOW;
            else
                flip_value->value += -nega_alpha_eval1(search, alpha, beta, false, searching) * W_NWS_VALUE;
        search->undo(&flip_value->flip);
        eval_undo(search, &flip_value->flip);
        return false;
    }
#endif

/*
    @brief Set the best move to the first element

    @param move_list            list of moves
    @param strt                 the first index
    @param siz                  the size of move_list
*/
inline void swap_next_best_move(std::vector<Flip_value> &move_list, const int strt, const int siz){
    int top_idx = strt;
    int best_value = move_list[strt].value;
    for (int i = strt + 1; i < siz; ++i){
        if (best_value < move_list[i].value){
            best_value = move_list[i].value;
            top_idx = i;
        }
    }
    if (top_idx != strt)
        std::swap(move_list[strt], move_list[top_idx]);
}

/*
    @brief Evaluate all legal moves for midgame

    @param search               search information
    @param move_list            list of moves
    @param depth                remaining depth
    @param alpha                alpha value
    @param beta                 beta value
    @param is_end_search        search till the end?
    @param searching            flag for terminating this search
*/
inline void move_list_evaluate(Search *search, std::vector<Flip_value> &move_list, int depth, int alpha, int beta, bool is_end_search, const bool *searching){
    if (move_list.size() == 1)
        return;
    int eval_alpha = -std::min(SCORE_MAX, beta + MOVE_ORDERING_VALUE_OFFSET_BETA);
    int eval_beta = -std::max(-SCORE_MAX, alpha - MOVE_ORDERING_VALUE_OFFSET_ALPHA);
    int phase = get_move_ordering_phase(search->n_discs);
    int eval_depth = depth >> 3;
    if (depth >= 18)
        eval_depth += (depth - 16) >> 1;
    bool wipeout_found = false;
    for (Flip_value &flip_value: move_list){
        if (!wipeout_found)
            wipeout_found = move_evaluate(search, &flip_value, eval_alpha, eval_beta, eval_depth, searching, phase);
        else
            flip_value.value = -INF;
    }
}

/*
    @brief Evaluate all legal moves for endgame

    @param search               search information
    @param move_list            list of moves
*/
inline void move_list_evaluate_end(Search *search, std::vector<Flip_value> &move_list){
    if (move_list.size() == 1)
        return;
    bool wipeout_found = false;
    for (Flip_value &flip_value: move_list){
        if (!wipeout_found)
            wipeout_found = move_evaluate_end(search, &flip_value);
        else
            flip_value.value = -INF;
    }
}

/*
    @brief Evaluate all legal moves for midgame NWS

    @param search               search information
    @param move_list            list of moves
    @param depth                remaining depth
    @param alpha                alpha value (beta = alpha + 1)
    @param is_end_search        search till the end?
    @param searching            flag for terminating this search
*/
inline void move_list_evaluate_nws(Search *search, std::vector<Flip_value> &move_list, int depth, int alpha, bool is_end_search, const bool *searching){
    if (move_list.size() == 1)
        return;
    const int eval_alpha = -std::min(SCORE_MAX, alpha + MOVE_ORDERING_NWS_VALUE_OFFSET_BETA);
    const int eval_beta = -std::max(-SCORE_MAX, alpha - MOVE_ORDERING_NWS_VALUE_OFFSET_ALPHA);
    int phase = get_move_ordering_phase(search->n_discs);
    const int eval_depth = depth >> 4;
    bool wipeout_found = false;
    for (Flip_value &flip_value: move_list){
        if (!wipeout_found)
            wipeout_found = move_evaluate_nws(search, &flip_value, eval_alpha, eval_beta, eval_depth, searching, phase);
        else
            flip_value.value = -INF;
    }
}
