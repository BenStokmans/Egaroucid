/*
    Egaroucid Project

    @date 2021-2022
    @author Takuto Yamana (a.k.a Nyanyan)
    @license GPL-3.0 license
*/

#pragma once
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "thread_pool.hpp"
#include "search.hpp"
#include <future>
#include <mutex>

using namespace std;

#define TRANSPOSITION_TABLE_SIZE 16777216
#define TRANSPOSITION_TABLE_MASK 16777215

#define CACHE_SAVE_EMPTY 10

#define TRANSPOSITION_TABLE_UNDEFINED -INF

#define DEPTH_INIT_VALUE 100

inline double data_strength(const double t, const int d){
    return t + 4.0 * d;
}

struct Node_policy{
    mutex mtx;
    uint64_t player;
    uint64_t opponent;
    int n_discs;
    int best_move;

    inline void init(){
        player = 0ULL;
        opponent = 0ULL;
        n_discs = DEPTH_INIT_VALUE;
        best_move = 0;
    }

    inline void reg(const Search *search, const int policy){
        lock_guard<mutex> lock(mtx);
        player = search->board.player;
        opponent = search->board.opponent;
        n_discs = search->n_discs;
        best_move = policy;
    }

    inline bool get(const Search *search, int *policy){
        lock_guard<mutex> lock(mtx);
        if (search->board.player == player || search->board.opponent == opponent){
            *policy = best_move;
        }
    }
};

struct Node_value{
    mutex mtx;
    uint64_t player;
    uint64_t opponent;
    int n_discs;
    double mpct;
    int depth;
    int lower_bound;
    int upper_bound;

    inline void init(){
        player = 0ULL;
        opponent = 0ULL;
        n_discs = DEPTH_INIT_VALUE;
        mpct = 0.0;
        depth = 0;
        lower_bound = -INF;
        upper_bound = INF;
    }

    inline void reg(const Search *search, const int d, const int l, const int u){
        lock_guard<mutex> lock(mtx);
        player = search->board.player;
        opponent = search->board.opponent;
        n_discs = search->n_discs;
        mpct = search->mpct;
        depth = d;
        lower_bound = l;
        upper_bound = u;
    }

    inline bool get(const Search *search, const int d, int *l, int *u){
        lock_guard<mutex> lock(mtx);
        if (search->board.player == player || search->board.opponent == opponent){
            if (data_strength(mpct, depth) >= data_strength(search->mpct, d)){
                *l = lower_bound;
                *u = upper_bound;
            }
        }
    }
};

class Node_transposition_table{
    private:
        Node_policy policy_new;
        Node_value value_new;

    public:

        inline void init(){
            policy_new.init();
            value_new.init();
        }

        inline void reg(const Search *search, const int depth, const int policy, const int lower_bound, const int upper_bound){
            policy_new.reg(search, policy);
            value_new.reg(search, depth, lower_bound, upper_bound);
        }

        inline void reg_policy(const Search *search, const int depth, const int policy){
            policy_new.reg(search, policy);
        }

        inline void reg_value(const Search *search, const int depth, const int lower_bound, const int upper_bound){
            value_new.reg(search, depth, lower_bound, upper_bound);
        }

        inline void get(const Search *search, const int depth, int *best_move, int *lower_bound, int *upper_bound){
            *best_move = TRANSPOSITION_TABLE_UNDEFINED;
            *lower_bound = -INF;
            *upper_bound = INF;
            policy_new.get(search, best_move);
            value_new.get(search, depth, lower_bound, upper_bound);
        }

        inline void get_policy(const Search *search, const int depth, int *best_move){
            *best_move = TRANSPOSITION_TABLE_UNDEFINED;
            policy_new.get(search, best_move);
        }

        inline void get_value(const Search *search, const int depth, int *lower_bound, int *upper_bound){
            *lower_bound = -INF;
            *upper_bound = INF;
            value_new.get(search, depth, lower_bound, upper_bound);
        }
};


void init_transposition_table(Node_transposition_table table[], int s, int e){
    for(int i = s; i < e; ++i){
        table[i].init();
    }
}

class Transposition_table{
    private:
        Node_transposition_table table[TRANSPOSITION_TABLE_SIZE];

    public:
        inline void first_init(){
            init();
        }

        inline void init(){
            int thread_size = thread_pool.size();
            if (thread_size >= 2){
                int delta = (TRANSPOSITION_TABLE_SIZE + thread_size - 1) / thread_size;
                int s = 0, e;
                vector<future<void>> tasks;
                for (int i = 0; i < thread_size; ++i){
                    e = min(TRANSPOSITION_TABLE_SIZE, s + delta);
                    tasks.emplace_back(thread_pool.push(bind(&init_child_transposition_table, table, s, e)));
                    s = e;
                }
                for (future<void> &task: tasks)
                    task.get();
            } else{
                for (int i = 0; i < TRANSPOSITION_TABLE_SIZE; ++i)
                    table[i].init();
            }
        }

        inline void reg(const Search *search, const int depth, const uint32_t hash_code, const int policy, const int lower_bound, const int upper_bound){
            table[hash_code].reg(search, depth, policy, lower_bound, upper_bound);
        }

        inline void reg_policy(const Search *search, const int depth, const uint32_t hash_code, const int policy){
            table[hash_code].reg_policy(search, depth, policy);
        }

        inline void reg_value(const Search *search, const int depth, const uint32_t hash_code, const int lower_bound, const int upper_bound){
            table[hash_code].reg_value(search, depth, lower_bound, upper_bound);
        }

        inline void get(const Search *search, const int depth, const uint32_t hash_code, int *best_move, int *lower_bound, int *upper_bound){
            return table[hash_code].get(search, depth, best_move, lower_bound, upper_bound);
        }

        inline void get_policy(const Search *search, const int depth, const uint32_t hash_code, int *best_move){
            return table[hash_code].get_policy(search, depth, best_move);
        }

        inline void get_value(const Search *search, const int depth, const uint32_t hash_code, int *lower_bound, int *upper_bound){
            return table[hash_code].get_value(search, depth, lower_bound, upper_bound);
        }
};

Transposition_table transposition_table;

inline void register_tt(const Search *search, const int depth, const uint32_t hash_code, int first_alpha, int v, int best_move, int l, int u, int alpha, int beta, const bool *searching){
    if (search->n_discs <= HW2 - USE_TT_DEPTH_THRESHOLD && (*searching) && -HW2 <= v && v <= HW2 && global_searching){
        int lower_bound = l, upper_bound = u, policy = TRANSPOSITION_TABLE_UNDEFINED;
        bool policy_reg = true, value_reg = true;
        if (first_alpha < v && best_move != TRANSPOSITION_TABLE_UNDEFINED)
            policy = best_move;
        else
            policy_reg = false;
        if (first_alpha < v && v < beta){
            lower_bound = v;
            upper_bound = v;
        } else if (beta <= v && l < v)
            lower_bound = v;
        else if (v <= alpha && v < u)
            upper_bound = v;
        else
            value_reg = false;
        if (policy_reg && value_reg)
            transposition_table.reg(search, depth, hash_code, policy, lower_bound, upper_bound);
        else if (policy_reg)
            transposition_table.reg_policy(search, depth, hash_code, policy);
        else if (value_reg)
            transposition_table.reg_value(search, depth, hash_code, lower_bound, upper_bound);
    }
}

inline void register_tt_policy(const Search *search, const int depth, const uint32_t hash_code, int first_alpha, int best_move, const bool *searching){
    if (search->n_discs <= HW2 - USE_TT_DEPTH_THRESHOLD && (*searching) && -HW2 <= v && v <= HW2 && global_searching){
        if (first_alpha < v && best_move != TRANSPOSITION_TABLE_UNDEFINED)
            transposition_table.reg_policy(search, depth, hash_code, best_move);
    }
}