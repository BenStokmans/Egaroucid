#pragma once
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include <atomic>

#define TRANSPOSE_TABLE_SIZE 1048576
#define TRANSPOSE_TABLE_MASK 1048575

#define TRANSPOSE_TABLE_UNDEFINED -INF

class Node_child_transpose_table{
    public:
        atomic<bool> reg;
        atomic<unsigned long long> b;
        atomic<unsigned long long> w;
        atomic<int> p;
        atomic<int> values[HW2];
        atomic<bool> regs[HW2];
        atomic<int> best_move;
        atomic<int> best_value;

    public:
        inline void init(){
            reg = false;
            best_move = TRANSPOSE_TABLE_UNDEFINED;
            best_value = -INF;
            for (int i = 0; i < HW2; ++i)
                regs[i] = false;
        }

        inline void register_value(const Board *board, const int policy, const int value){
            init();
            reg = true;
            b = board->b;
            w = board->w;
            p = board->p;
            values[policy] = value;
            if (best_value < value){
                best_value = value;
                best_move = policy;
            }
        }

        inline void register_value(const int policy, const int value){
            values[policy] = value;
            if (best_value < value){
                best_value = value;
                best_move = policy;
            }
        }

        inline void get(int v[], int *b){
            *b = best_move;
            for (int i = 0; i < HW2; ++i){
                if (regs[i])
                    v[i] = values[i];
                else
                    v[i] = TRANSPOSE_TABLE_UNDEFINED;
            }
        }
};

class Child_transpose_table{
    private:
        int prev;
        int now;
        Node_child_transpose_table table[2][TRANSPOSE_TABLE_SIZE];

    public:
        inline void init(){
            now = 0;
            prev = 1;
            init_now();
            init_prev();
        }

        inline void ready_next_search(){
            swap(now, prev);
            init_now();
        }

        inline void init_prev(){
            for(int i = 0; i < TRANSPOSE_TABLE_SIZE; ++i)
                table[prev][i].init();
        }

        inline void init_now(){
            for(int i = 0; i < TRANSPOSE_TABLE_SIZE; ++i)
                table[now][i].init();
        }

        inline void reg(const Board *board, const int hash, const int policy, const int value){
            if (!table[now][hash].reg)
                table[now][hash].register_value(board, policy, value);
            else if (!compare_key(board, &table[now][hash]))
                table[now][hash].register_value(board, policy, value);
            else
                table[now][hash].register_value(policy, value);
        }

        inline bool get_now(Board *board, const int hash, int values[], int *best_move){
            if (table[now][hash].reg){
                if (compare_key(board, &table[now][hash])){
					table[now][hash].get(values, best_move);
                    return true;
                }
            }
            return false;
        }

        inline bool get_prev(Board *board, const int hash, int values[], int *best_move){
            if (table[prev][hash].reg){
                if (compare_key(board, &table[prev][hash])){
					table[prev][hash].get(values, best_move);
                    return true;
                }
            }
            return false;
        }

    private:
        inline bool compare_key(const Board *a, Node_child_transpose_table *b){
            return a->p == b->p && a->b == b->b && a->w == b->w;
        }
};


class Node_parent_transpose_table{
    public:
        atomic<bool> reg;
        atomic<unsigned long long> b;
        atomic<unsigned long long> w;
        atomic<int> p;
        atomic<int> lower;
        atomic<int> upper;

    public:
        inline void init(){
            reg = false;
            lower = -INF;
            upper = INF;
        }

        inline void register_value(const Board *board, const int l, const int u){
            init();
            reg = true;
            b = board->b;
            w = board->w;
            p = board->p;
            lower = l;
            upper = u;
        }

        inline void register_value(const int l, const int u){
            lower = l;
            upper = u;
        }

        inline void get(int *l, int *u){
            *l = lower;
            *u = upper;
        }
};

class Parent_transpose_table{
    private:
        int prev;
        int now;
        Node_parent_transpose_table table[2][TRANSPOSE_TABLE_SIZE];

    public:
        inline void init(){
            now = 0;
            prev = 1;
            init_now();
            init_prev();
        }

        inline void ready_next_search(){
            swap(now, prev);
            init_now();
        }

        inline void init_prev(){
            for(int i = 0; i < TRANSPOSE_TABLE_SIZE; ++i)
                table[prev][i].init();
        }

        inline void init_now(){
            for(int i = 0; i < TRANSPOSE_TABLE_SIZE; ++i)
                table[now][i].init();
        }

        inline void reg(const Board *board, const int hash, const int l, const int u){
            if (!table[now][hash].reg)
                table[now][hash].register_value(board, l, u);
            else if (!compare_key(board, &table[now][hash]))
                table[now][hash].register_value(board, l, u);
            else
                table[now][hash].register_value(l, u);
        }

        inline void get_now(Board *board, const int hash, int *l, int *u){
            if (table[now][hash].reg){
                if (compare_key(board, &table[now][hash])){
					table[now][hash].get(l, u);
                    return;
                }
            }
            *l = -INF;
            *u = INF;
        }

        inline void get_prev(Board *board, const int hash, int *l, int *u){
            if (table[prev][hash].reg){
                if (compare_key(board, &table[prev][hash])){
					table[prev][hash].get(l, u);
                    return;
                }
            }
            *l = -INF;
            *u = INF;
        }

    private:
        inline bool compare_key(const Board *a, Node_parent_transpose_table *b){
            return a->p == b->p && a->b == b->b && a->w == b->w;
        }
};
