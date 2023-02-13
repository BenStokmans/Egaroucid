/*
    Egaroucid Project

    @file stability.hpp
        Calculate stable discs
    @date 2021-2023
    @author Takuto Yamana (a.k.a. Nyanyan)
    @license GPL-3.0 license
    @notice I referred to codes written by others
*/

#pragma once
#include <iostream>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "search.hpp"
#include "util.hpp"

/*
    @brief Pre-calculation result of edge stability
*/
uint64_t stability_edge_arr[N_8BIT][N_8BIT][2];

/*
    @brief Find flippable discs on a line

    @param p                    an integer representing player
    @param o                    an integer representing opponent
    @param place                place to put a disc
    @param np                   an integer to store result of player discs
    @param no                   an integer to store result of opponent discs
*/
inline void probably_move_line(int p, int o, int place, int *np, int *no){
    int i, j;
    *np = p | (1 << place);
    for (i = place - 1; i > 0 && (1 & (o >> i)); --i);
    if (1 & (p >> i)){
        for (j = place - 1; j > i; --j)
            *np ^= 1 << j;
    }
    for (i = place + 1; i < HW_M1 && (1 & (o >> i)); ++i);
    if (1 & (p >> i)){
        for (j = place + 1; j < i; ++j)
            *np ^= 1 << j;
    }
    *no = o & ~(*np);
}

/*
    @brief Calculate all stable discs on a line

    @param b                    player 1/2
    @param w                    player 2/2
*/
int calc_stability_line(int b, int w){
    int i, nb, nw, res = b | w;
    int empties = ~(b | w);
    for (i = 0; i < HW; ++i){
        if (1 & (empties >> i)){
            probably_move_line(b, w, i, &nb, &nw);
            res &= b | nw;
            res &= calc_stability_line(nb, nw);
            probably_move_line(w, b, i, &nw, &nb);
            res &= w | nb;
            res &= calc_stability_line(nb, nw);
        }
    }
    return res;
}

/*
    @brief Initialize stability calculation
*/
inline void stability_init() {
    int place, b, w, stab;
    for (b = 0; b < N_8BIT; ++b) {
        for (w = b; w < N_8BIT; ++w){
            if (b & w){
                stability_edge_arr[b][w][0] = 0;
                stability_edge_arr[b][w][1] = 0;
                stability_edge_arr[w][b][0] = 0;
                stability_edge_arr[w][b][1] = 0;
            } else{
                stab = calc_stability_line(b, w);
                stability_edge_arr[b][w][0] = 0;
                stability_edge_arr[b][w][1] = 0;
                for (place = 0; place < HW; ++place){
                    if (1 & (stab >> place)){
                        stability_edge_arr[b][w][0] |= 1ULL << place;
                        stability_edge_arr[b][w][1] |= 1ULL << (place * HW);
                    }
                }
                stability_edge_arr[w][b][0] = stability_edge_arr[b][w][0];
                stability_edge_arr[w][b][1] = stability_edge_arr[b][w][1];
            }
        }
    }
}

// @notice from http://www.amy.hi-ho.ne.jp/okuhara/bitboard.htm
// modified by Nyanyan
#if USE_SIMD
    inline void full_stability(uint64_t discs, uint64_t *h, uint64_t *v, uint64_t *d7, uint64_t *d9){
        // horizontal & vertical
        const __m128i shift0 = _mm_set_epi64x(1, 8);
        const __m128i shift1 = _mm_set_epi64x(2, 16);
        const __m128i shift2 = _mm_set_epi64x(4, 32);
        const __m128i hvmask = _mm_set_epi64x(0x0101010101010101ULL, 0x00000000000000FFULL);
        const __m128i hvmul = _mm_set_epi64x(0x00000000000000FFULL, 0x0101010101010101ULL);
        __m128i hv = _mm_set1_epi64x(discs);
        hv = _mm_and_si128(hv, _mm_srlv_epi64(hv, shift0));
        hv = _mm_and_si128(hv, _mm_srlv_epi64(hv, shift1));
        hv = _mm_and_si128(hv, _mm_srlv_epi64(hv, shift2));
        hv = _mm_and_si128(hv, hvmask);
        hv = _mm_mullo_epi64(hv, hvmul);
        *v = _mm_cvtsi128_si64(hv);
        *h = _mm_cvtsi128_si64(_mm_unpackhi_epi64(hv, hv));

        // diagonal
        const __m128i e790 = _mm_set1_epi64x(0xFF80808080808080);
        const __m128i e791 = _mm_set1_epi64x(0x01010101010101FF);
        const __m128i e792 = _mm_set1_epi64x(0x00003F3F3F3F3F3F);
        const __m128i e793 = _mm_set1_epi64x(0x0F0F0F0Ff0F0F0F0);
        __m128i l79, r79;
        l79 = r79 = _mm_unpacklo_epi64(_mm_cvtsi64_si128(discs), _mm_cvtsi64_si128(vertical_mirror(discs)));
        l79 = _mm_and_si128(l79, _mm_or_si128(e790, _mm_srli_epi64(l79, 9)));
        r79 = _mm_and_si128(r79, _mm_or_si128(e791, _mm_slli_epi64(r79, 9)));
        l79 = _mm_andnot_si128(_mm_andnot_si128(_mm_srli_epi64(l79, 18), e792), l79);
        r79 = _mm_andnot_si128(_mm_slli_epi64(_mm_andnot_si128(r79, e792), 18), r79);
        l79 = _mm_and_si128(_mm_and_si128(l79, r79), _mm_or_si128(e793,
            _mm_or_si128(_mm_srli_epi64(l79, 36), _mm_slli_epi64(r79, 36))));
        *d9 = _mm_cvtsi128_si64(l79);
        *d7 = vertical_mirror(_mm_cvtsi128_si64(_mm_unpackhi_epi64(l79, l79)));
    }
#else
    /*
        @brief Calculate full stability in horizontal direction

        @param full                 a bitboard representing discs
    */
    inline uint64_t full_stability_h(uint64_t full){
        full &= full >> 1;
        full &= full >> 2;
        full &= full >> 4;
        return (full & 0x0101010101010101ULL) * 0xFF;
    }

    /*
        @brief Calculate full stability in vertical direction

        @param full                 a bitboard representing discs
    */
    inline uint64_t full_stability_v(uint64_t full){
        full &= (full >> 8) | (full << 56);
        full &= (full >> 16) | (full << 48);
        full &= (full >> 32) | (full << 32);
        return full;
    }

    /*
        @brief Calculate full stability in diagonal direction

        @param full                 a bitboard representing discs
        @param full_d7              an integer to store result of d7 line
        @param full_d9              an integer to store result of d9 line
    */
    inline void full_stability_d(uint64_t full, uint64_t *full_d7, uint64_t *full_d9){
        constexpr uint64_t edge = 0xFF818181818181FFULL;
        uint64_t l7, r7, l9, r9;
        l7 = r7 = full;
        l7 &= edge | (l7 >> 7);        r7 &= edge | (r7 << 7);
        l7 &= 0xFFFF030303030303ULL | (l7 >> 14);    r7 &= 0xC0C0C0C0C0C0FFFFULL | (r7 << 14);
        l7 &= 0xFFFFFFFF0F0F0F0FULL | (l7 >> 28);    r7 &= 0xF0F0F0F0FFFFFFFFULL | (r7 << 28);
        *full_d7 = l7 & r7;

        l9 = r9 = full;
        l9 &= edge | (l9 >> 9);        r9 &= edge | (r9 << 9);
        l9 &= 0xFFFFC0C0C0C0C0C0ULL | (l9 >> 18);    r9 &= 0x030303030303FFFFULL | (r9 << 18);
        *full_d9 = l9 & r9 & (0x0F0F0F0FF0F0F0F0ULL | (l9 >> 36) | (r9 << 36));
    }

    /*
        @brief Calculate full stability in all direction

        @param discs                a bitboard representing discs
        @param h                    an integer to store result of h line
        @param v                    an integer to store result of h line
        @param d7                   an integer to store result of d7 line
        @param d9                   an integer to store result of d9 line
    */
    inline void full_stability(uint64_t discs, uint64_t *h, uint64_t *v, uint64_t *d7, uint64_t *d9){
        *h = full_stability_h(discs);
        *v = full_stability_v(discs);
        full_stability_d(discs, d7, d9);
    }
#endif
// end of modification

/*
    @brief Calculate stable discs as a bitboard

    This function cannot find every stable discs
    From an idea of https://github.com/abulmo/edax-reversi/blob/1ae7c9fe5322ac01975f1b3196e788b0d25c1e10/src/board.c#L1030

    @param board                board
    @return found stable discs as a bitboard
*/
#if USE_SIMD
    inline void calc_stability_each_bits(Board *board, uint64_t *res_player, uint64_t *res_opponent){
        uint64_t full_h, full_v, full_d7, full_d9;
        uint64_t edge_stability = 0, player_stability = 0, opponent_stability = 0, n_stability;
        uint64_t h, v, d7, d9;
        const uint64_t player_mask = board->player & 0x007E7E7E7E7E7E00ULL;
        const uint64_t opponent_mask = board->opponent & 0x007E7E7E7E7E7E00ULL;
        uint8_t pl, op;
        pl = 0b11111111U & board->player;
        op = 0b11111111U & board->opponent;
        edge_stability |= stability_edge_arr[pl][op][0];
        pl = join_h_line(board->player, 7);
        op = join_h_line(board->opponent, 7);
        edge_stability |= stability_edge_arr[pl][op][0] << 56;
        pl = join_v_line(board->player, 0);
        op = join_v_line(board->opponent, 0);
        edge_stability |= stability_edge_arr[pl][op][1];
        pl = join_v_line(board->player, 7);
        op = join_v_line(board->opponent, 7);
        edge_stability |= stability_edge_arr[pl][op][1] << 7;
        full_stability(board->player | board->opponent, &full_h, &full_v, &full_d7, &full_d9);

        n_stability = (edge_stability & board->player) | (full_h & full_v & full_d7 & full_d9 & player_mask);
        while (n_stability & ~player_stability){
            player_stability |= n_stability;
            h = (player_stability >> 1) | (player_stability << 1) | full_h;
            v = (player_stability >> HW) | (player_stability << HW) | full_v;
            d7 = (player_stability >> HW_M1) | (player_stability << HW_M1) | full_d7;
            d9 = (player_stability >> HW_P1) | (player_stability << HW_P1) | full_d9;
            n_stability = h & v & d7 & d9 & player_mask;
        }

        n_stability = (edge_stability & board->opponent) | (full_h & full_v & full_d7 & full_d9 & opponent_mask);
        while (n_stability & ~opponent_stability){
            opponent_stability |= n_stability;
            h = (opponent_stability >> 1) | (opponent_stability << 1) | full_h;
            v = (opponent_stability >> HW) | (opponent_stability << HW) | full_v;
            d7 = (opponent_stability >> HW_M1) | (opponent_stability << HW_M1) | full_d7;
            d9 = (opponent_stability >> HW_P1) | (opponent_stability << HW_P1) | full_d9;
            n_stability = h & v & d7 & d9 & opponent_mask;
        }

        *res_player = player_stability;
        *res_opponent = opponent_stability;
    }
#else
    inline void calc_stability_each_bits(Board *board, uint64_t *res_player, uint64_t *res_opponent){
        uint64_t full_h, full_v, full_d7, full_d9;
        uint64_t edge_stability = 0, player_stability = 0, opponent_stability = 0, n_stability;
        uint64_t h, v, d7, d9;
        const uint64_t player_mask = board->player & 0x007E7E7E7E7E7E00ULL;
        const uint64_t opponent_mask = board->opponent & 0x007E7E7E7E7E7E00ULL;
        uint8_t pl, op;
        pl = 0b11111111U & board->player;
        op = 0b11111111U & board->opponent;
        edge_stability |= stability_edge_arr[pl][op][0];
        pl = join_h_line(board->player, 7);
        op = join_h_line(board->opponent, 7);
        edge_stability |= stability_edge_arr[pl][op][0] << 56;
        pl = join_v_line(board->player, 0);
        op = join_v_line(board->opponent, 0);
        edge_stability |= stability_edge_arr[pl][op][1];
        pl = join_v_line(board->player, 7);
        op = join_v_line(board->opponent, 7);
        edge_stability |= stability_edge_arr[pl][op][1] << 7;
        full_stability(board->player | board->opponent, &full_h, &full_v, &full_d7, &full_d9);

        n_stability = (edge_stability & board->player) | (full_h & full_v & full_d7 & full_d9 & player_mask);
        while (n_stability & ~player_stability){
            player_stability |= n_stability;
            h = (player_stability >> 1) | (player_stability << 1) | full_h;
            v = (player_stability >> HW) | (player_stability << HW) | full_v;
            d7 = (player_stability >> HW_M1) | (player_stability << HW_M1) | full_d7;
            d9 = (player_stability >> HW_P1) | (player_stability << HW_P1) | full_d9;
            n_stability = h & v & d7 & d9 & player_mask;
        }

        n_stability = (edge_stability & board->opponent) | (full_h & full_v & full_d7 & full_d9 & opponent_mask);
        while (n_stability & ~opponent_stability){
            opponent_stability |= n_stability;
            h = (opponent_stability >> 1) | (opponent_stability << 1) | full_h;
            v = (opponent_stability >> HW) | (opponent_stability << HW) | full_v;
            d7 = (opponent_stability >> HW_M1) | (opponent_stability << HW_M1) | full_d7;
            d9 = (opponent_stability >> HW_P1) | (opponent_stability << HW_P1) | full_d9;
            n_stability = h & v & d7 & d9 & opponent_mask;
        }

        *res_player = player_stability;
        *res_opponent = opponent_stability;
    }
#endif

inline uint64_t calc_stability_bits(Board *board){
    uint64_t p, o;
    calc_stability_each_bits(board, &p, &o);
    return p | o;
}

/*
    @brief Calculate number of stable discs

    This function cannot find every stable discs
    From an idea of https://github.com/abulmo/edax-reversi/blob/1ae7c9fe5322ac01975f1b3196e788b0d25c1e10/src/board.c#L1030

    @param board                board
    @param stab0                number of player's stable discs
    @param stab1                number of opponent's stable discs
*/
inline void calc_stability(Board *board, int *stab0, int *stab1){
    uint64_t p, o;
    calc_stability_each_bits(board, &p, &o);
    *stab0 = pop_count_ull(p);
    *stab1 = pop_count_ull(o);
}

/*
    @brief Stability cutoff

    If P (number of player's stable discs) and O (number of opponent's stable discs) are calculated, 
    then the final score should be 2 * P - 64 <= final_score <= 64 - 2 * O.
    Using this formula, we can narrow the search window.

    @param search               search information
    @param alpha                alpha value
    @param beta                 beta value
    @return SCORE_UNDEFINED if no cutoff found else the score
*/
inline int stability_cut(Search *search, int *alpha, int *beta){
    if (*alpha <= nws_stability_threshold[search->n_discs]){
        int stab_player, stab_opponent;
        calc_stability(&search->board, &stab_player, &stab_opponent);
        int n_alpha = 2 * stab_player - HW2;
        int n_beta = HW2 - 2 * stab_opponent;
        if (*beta <= n_alpha)
            return n_alpha;
        if (n_beta <= *alpha)
            return n_beta;
        *alpha = std::max(*alpha, n_alpha);
        *beta = std::min(*beta, n_beta);
    }
    return SCORE_UNDEFINED;
}

/*
    @brief Stability cutoff for NWS (Null Window Search)

    If P (number of player's stable discs) and O (number of opponent's stable discs) are calculated, 
    then the final score should be 2 * P - 64 <= final_score <= 64 - 2 * O.
    Using this formula, we can narrow the search window.

    @param search               search information
    @param alpha                alpha value (beta = alpha + 1)
    @return SCORE_UNDEFINED if no cutoff found else the score
*/
inline int stability_cut_nws(Search *search, int *alpha){
    if (*alpha <= nws_stability_threshold[search->n_discs]){
        int stab_player, stab_opponent;
        calc_stability(&search->board, &stab_player, &stab_opponent);
        int n_alpha = 2 * stab_player - HW2;
        int n_beta = HW2 - 2 * stab_opponent;
        if (*alpha < n_alpha)
            return n_alpha;
        if (n_beta <= *alpha)
            return n_beta;
    }
    return SCORE_UNDEFINED;
}