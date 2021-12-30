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

#define simple_mid_threshold 3
#define simple_end_threshold 7

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
#if USE_MID_SMOOTH
    const double mpcsd[n_phases][mpc_max_depth - mpc_min_depth + 1]={
        {100, 100, 101, 118, 140, 152, 155, 169},
        {152, 138, 130, 218, 247, 205, 251, 228},
        {227, 237, 197, 286, 205, 182, 255, 210},
        {289, 232, 230, 272, 238, 219, 389, 360},
        {283, 294, 230, 307, 410, 258, 362, 338},
        {345, 316, 245, 427, 418, 361, 395, 424},
        {357, 321, 256, 448, 346, 265, 508, 510},
        {342, 324, 292, 484, 410, 541, 562, 547},
        {412, 389, 393, 803, 602, 454, 610, 648},
        {361, 270, 330, 426, 338, 243, 324, 286}
    };
    const double mpcsd_final[mpc_max_depth_final - mpc_min_depth_final + 1] = {
        545, 526, 591, 579, 573, 591, 565, 541, 548, 539, 526, 528, 518, 553, 543, 532, 580, 578, 584, 642
    };
#else
    const double mpcsd[n_phases][mpc_max_depth - mpc_min_depth + 1]={
        {140, 94, 114, 130, 137, 143, 159, 179},
        {187, 155, 201, 238, 246, 211, 249, 259},
        {318, 266, 218, 267, 245, 177, 240, 224},
        {287, 225, 235, 233, 257, 236, 346, 329},
        {320, 308, 302, 446, 294, 278, 419, 354},
        {325, 353, 311, 398, 366, 450, 467, 365},
        {350, 330, 330, 483, 353, 342, 516, 482},
        {371, 388, 369, 442, 465, 427, 564, 611},
        {453, 487, 350, 537, 629, 581, 608, 733},
        {396, 326, 262, 400, 297, 218, 321, 151}
    };
    const double mpcsd_final[mpc_max_depth_final - mpc_min_depth_final + 1] = {
        532, 503, 616, 599, 592, 581, 554, 521, 571, 565, 554, 519, 510, 574, 568, 554, 568, 566, 573, 638
    };
#endif
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
