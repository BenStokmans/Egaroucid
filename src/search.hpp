#pragma once
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "evaluate.hpp"
#include "transpose_table.hpp"

using namespace std;

#define search_epsilon 1
constexpr int cache_hit = step * 100;
constexpr int cache_both = step * 100;
constexpr int parity_vacant_bonus = step * 10;
constexpr int canput_bonus = step / 10;
constexpr int mtd_threshold = step * 4;
constexpr int mtd_end_threshold = step * 5;

#define mpc_min_depth 3
#define mpc_max_depth 10
#define mpc_min_depth_final 9
#define mpc_max_depth_final 28
#define mpct_final 2.4
#if BOOK_MODE
    #define mpct 0.3
#else
    #define mpct 1.6
#endif

#define simple_mid_threshold 3
#define simple_end_threshold 8

#define po_max_depth 8

#define extra_stability_threshold 58

#define ybwc_mid_first_num 1
#define ybwc_end_first_num 2
#define multi_thread_depth 1

const int cell_weight[hw2] = {
    10, 3, 9, 7, 7, 9, 3, 10, 
    3, 2, 4, 5, 5, 4, 2, 3, 
    9, 4, 8, 6, 6, 8, 4, 9, 
    7, 5, 6, 0, 0, 6, 5, 7, 
    7, 5, 6, 0, 0, 6, 5, 7, 
    9, 4, 8, 6, 6, 8, 4, 9, 
    3, 2, 4, 5, 5, 4, 2, 3, 
    10, 3, 9, 7, 7, 9, 3, 10
};

const int mpcd[30] = {0, 1, 0, 1, 2, 3, 2, 3, 4, 3, 4, 3, 4, 5, 4, 5, 6, 5, 6, 7, 6, 7, 6, 7, 8, 7, 8, 9, 8, 9};
const double mpcsd[n_phases][mpc_max_depth - mpc_min_depth + 1]={
    {183, 116, 134, 177, 228, 162, 176, 140},
    {305, 233, 262, 303, 233, 252, 303, 297},
    {303, 208, 233, 231, 211, 177, 235, 257},
    {314, 308, 290, 289, 358, 302, 397, 529},
    {336, 308, 403, 427, 402, 470, 428, 390},
    {535, 536, 408, 469, 377, 317, 499, 469},
    {371, 338, 402, 333, 304, 282, 435, 536},
    {503, 450, 458, 377, 427, 411, 565, 394},
    {430, 392, 338, 516, 352, 402, 568, 469},
    {448, 341, 344, 431, 404, 385, 484, 421},
    {465, 402, 411, 451, 548, 495, 652, 627},
    {372, 430, 422, 572, 542, 622, 587, 599},
    {430, 419, 369, 585, 554, 480, 699, 634},
    {456, 375, 453, 584, 531, 485, 612, 394},
    {312, 253, 169, 286, 178, 0, 150, 0}
};
const double mpcsd_final[mpc_max_depth_final - mpc_min_depth_final + 1] = {
    505, 481, 540, 512, 489, 506, 485, 454, 491, 467, 450, 444, 425, 452, 438, 428, 409, 396, 388, 432
};
unsigned long long can_be_flipped[hw2];

unsigned long long searched_nodes;
vector<int> vacant_lst;

struct search_result{
    int policy;
    int value;
    int depth;
    int nps;
};

inline void search_init(){
    int i;
    for (int cell = 0; cell < hw2; ++cell){
        can_be_flipped[cell] = 0b1111111110000001100000011000000110000001100000011000000111111111;
        for (i = 0; i < hw; ++i){
            if (global_place[place_included[cell][0]][i] != -1)
                can_be_flipped[cell] |= 1ULL << global_place[place_included[cell][0]][i];
        }
        for (i = 0; i < hw; ++i){
            if (global_place[place_included[cell][1]][i] != -1)
                can_be_flipped[cell] |= 1ULL << global_place[place_included[cell][1]][i];
        }
        for (i = 0; i < hw; ++i){
            if (global_place[place_included[cell][2]][i] != -1)
                can_be_flipped[cell] |= 1ULL << global_place[place_included[cell][2]][i];
        }
        if (place_included[cell][3] != -1){
            for (i = 0; i < hw; ++i){
                if (global_place[place_included[cell][3]][i] != -1)
                    can_be_flipped[cell] |= 1ULL << global_place[place_included[cell][3]][i];
            }
        }
    }
    cerr << "search initialized" << endl;
}

int cmp_vacant(int p, int q){
    return cell_weight[p] > cell_weight[q];
}

inline void move_ordering(board *b){
    int l, u;
    transpose_table.get_prev(b, b->hash() & search_hash_mask, &l, &u);
    if (u != inf && l != -inf)
        b->v = (u + l) / 2 + cache_hit + cache_both;
    else if (u != inf)
        b->v += u + cache_hit;
    else if (l != -inf)
        b->v += l + cache_hit;
    else
        b->v = -mid_evaluate(b);
}

inline void move_ordering_eval(board *b){
    b->v = -mid_evaluate(b);
}

/*
inline int calc_xx_stability(board *b, int p){
    return
        (pop_digit[b->b[1]][2] == p && pop_digit[b->b[0]][2] == p && pop_digit[b->b[0]][1] == p && pop_digit[b->b[1]][0] == p && pop_digit[b->b[1]][1] == p && (pop_digit[b->b[0]][3] == p || (pop_digit[b->b[2]][1] == p && pop_digit[b->b[3]][0] == p) || (pop_digit[b->b[0]][3] != vacant && pop_digit[b->b[2]][1] != vacant && pop_digit[b->b[3]][0] != vacant))) + 
        (pop_digit[b->b[1]][5] == p && pop_digit[b->b[0]][5] == p && pop_digit[b->b[0]][6] == p && pop_digit[b->b[1]][7] == p && pop_digit[b->b[1]][6] == p && (pop_digit[b->b[0]][4] == p || (pop_digit[b->b[2]][6] == p && pop_digit[b->b[3]][7] == p) || (pop_digit[b->b[0]][4] != vacant && pop_digit[b->b[2]][6] != vacant && pop_digit[b->b[3]][7] != vacant))) + 
        (pop_digit[b->b[6]][2] == p && pop_digit[b->b[7]][2] == p && pop_digit[b->b[7]][1] == p && pop_digit[b->b[6]][0] == p && pop_digit[b->b[6]][1] == p && (pop_digit[b->b[7]][3] == p || (pop_digit[b->b[5]][1] == p && pop_digit[b->b[4]][0] == p) || (pop_digit[b->b[7]][3] != vacant && pop_digit[b->b[5]][1] != vacant && pop_digit[b->b[4]][0] != vacant))) + 
        (pop_digit[b->b[6]][5] == p && pop_digit[b->b[7]][5] == p && pop_digit[b->b[7]][6] == p && pop_digit[b->b[6]][7] == p && pop_digit[b->b[6]][6] == p && (pop_digit[b->b[7]][4] == p || (pop_digit[b->b[5]][6] == p && pop_digit[b->b[4]][7] == p) || (pop_digit[b->b[7]][4] != vacant && pop_digit[b->b[5]][6] != vacant && pop_digit[b->b[4]][7] != vacant))) + 
        (pop_digit[b->b[9]][2] == p && pop_digit[b->b[8]][2] == p && pop_digit[b->b[8]][1] == p && pop_digit[b->b[9]][0] == p && pop_digit[b->b[9]][1] == p && (pop_digit[b->b[8]][3] == p || (pop_digit[b->b[10]][1] == p && pop_digit[b->b[11]][0] == p) || (pop_digit[b->b[8]][3] != vacant && pop_digit[b->b[10]][1] != vacant && pop_digit[b->b[11]][0] != vacant))) + 
        (pop_digit[b->b[9]][5] == p && pop_digit[b->b[8]][5] == p && pop_digit[b->b[8]][6] == p && pop_digit[b->b[9]][7] == p && pop_digit[b->b[9]][6] == p && (pop_digit[b->b[8]][4] == p || (pop_digit[b->b[10]][6] == p && pop_digit[b->b[11]][7] == p) || (pop_digit[b->b[8]][4] != vacant && pop_digit[b->b[10]][6] != vacant && pop_digit[b->b[11]][7] != vacant))) + 
        (pop_digit[b->b[14]][2] == p && pop_digit[b->b[15]][2] == p && pop_digit[b->b[15]][1] == p && pop_digit[b->b[14]][0] == p && pop_digit[b->b[14]][1] == p && (pop_digit[b->b[15]][3] == p || (pop_digit[b->b[13]][1] == p && pop_digit[b->b[12]][0] == p) || (pop_digit[b->b[15]][3] != vacant && pop_digit[b->b[13]][1] != vacant && pop_digit[b->b[12]][0] != vacant))) + 
        (pop_digit[b->b[14]][5] == p && pop_digit[b->b[15]][5] == p && pop_digit[b->b[15]][6] == p && pop_digit[b->b[14]][7] == p && pop_digit[b->b[14]][6] == p && (pop_digit[b->b[15]][4] == p || (pop_digit[b->b[13]][6] == p && pop_digit[b->b[12]][7] == p) || (pop_digit[b->b[15]][4] != vacant && pop_digit[b->b[13]][6] != vacant && pop_digit[b->b[12]][7] != vacant)));
}

inline int calc_x_stability(board *b, int p){
    return
        (pop_digit[b->b[1]][1] == p && (pop_digit[b->b[0]][2] == p || pop_digit[b->b[2]][0] == p || (pop_digit[b->b[0]][2] != vacant || pop_digit[b->b[2]][0] != vacant)) && pop_digit[b->b[0]][1] == p && pop_digit[b->b[1]][0] == p && pop_digit[b->b[0]][0] == p) + 
        (pop_digit[b->b[1]][6] == p && (pop_digit[b->b[0]][5] == p || pop_digit[b->b[2]][7] == p || (pop_digit[b->b[0]][5] != vacant || pop_digit[b->b[2]][7] != vacant)) && pop_digit[b->b[0]][6] == p && pop_digit[b->b[1]][7] == p && pop_digit[b->b[0]][7] == p) + 
        (pop_digit[b->b[6]][1] == p && (pop_digit[b->b[7]][2] == p || pop_digit[b->b[5]][0] == p || (pop_digit[b->b[7]][2] != vacant || pop_digit[b->b[5]][0] != vacant)) && pop_digit[b->b[7]][1] == p && pop_digit[b->b[6]][0] == p && pop_digit[b->b[7]][0] == p) + 
        (pop_digit[b->b[6]][6] == p && (pop_digit[b->b[7]][5] == p || pop_digit[b->b[5]][7] == p || (pop_digit[b->b[7]][5] != vacant || pop_digit[b->b[5]][7] != vacant)) && pop_digit[b->b[7]][6] == p && pop_digit[b->b[6]][7] == p && pop_digit[b->b[7]][7] == p);
}
*/

inline int calc_stability(board *b, int p){
    return
        stability_edge_arr[p][b->b[0]] + stability_edge_arr[p][b->b[7]] + stability_edge_arr[p][b->b[8]] + stability_edge_arr[p][b->b[15]] + 
        stability_corner_arr[p][b->b[0]] + stability_corner_arr[p][b->b[7]]; // + 
        //calc_x_stability(b, p); // + calc_xx_stability(b, p);
}

inline void calc_extra_stability(board *b, int p, unsigned long long extra_stability, int *pres, int *ores){
    *pres = 0;
    *ores = 0;
    int y, x;
    extra_stability >>= hw;
    for (y = 1; y < hw_m1; ++y){
        extra_stability >>= 1;
        for (x = 1; x < hw_m1; ++x){
            if ((extra_stability & 1) == 0){
                if (pop_digit[b->b[y]][x] == p)
                    ++*pres;
                else if (pop_digit[b->b[y]][x] == 1 - p)
                    ++*ores;
            }
            extra_stability >>= 1;
        }
        extra_stability >>= 1;
    }
}

inline unsigned long long calc_extra_stability_ull(board *b){
    unsigned long long extra_stability = 0b1111111110000001100000011000000110000001100000011000000111111111;
    for (const int &cell: vacant_lst){
        if (pop_digit[b->b[cell / hw]][cell % hw] == vacant)
            extra_stability |= can_be_flipped[cell];
    }
    return extra_stability;
}

inline bool stability_cut(board *b, int *alpha, int *beta){
    if (b->n >= extra_stability_threshold){
        int ps, os;
        calc_extra_stability(b, b->p, calc_extra_stability_ull(b), &ps, &os);
        *alpha = max(*alpha, step * (2 * (calc_stability(b, b->p) + ps) - hw2));
        *beta = min(*beta, step * (hw2 - 2 * (calc_stability(b, 1 - b->p) + os)));
    } else{
        *alpha = max(*alpha, step * (2 * calc_stability(b, b->p) - hw2));
        *beta = min(*beta, step * (hw2 - 2 * calc_stability(b, 1 - b->p)));
    }
    return *alpha >= *beta;
}

inline int calc_canput_exact(board *b){
    int res = 0;
    for (const int &cell: vacant_lst)
        res += b->legal(cell);
    return res;
}
