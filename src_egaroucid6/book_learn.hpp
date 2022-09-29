#pragma once
#include <iostream>
#include <unordered_set>
#include "evaluate.hpp"
#include "board.hpp"
#include "ai.hpp"

#define AUTO_BOOK_SAVE_TIME 60000

using namespace std;

Search_result ai(Board board, int level, bool use_book, bool use_multi_thread, bool show_log);
int ai_nws(Board board, int level, int alpha, int beta, bool use_multi_thread);

inline int book_learn_calc_value(Board board, int level){
    return ai(board, level, true, true, false).value;
}

int book_learn_search(Board board, int level, const int book_depth, int error_sum, int expected_error, const int adopt_error_sum, Board *board_copy, int *player, uint64_t *strt_tim, string book_file, string book_bak){
    if (!global_searching)
        return SCORE_UNDEFINED;
    if (error_sum > adopt_error_sum)
        return SCORE_UNDEFINED;
    if (tim() - *strt_tim > AUTO_BOOK_SAVE_TIME){
        book.save_bin(book_file, book_bak);
        *strt_tim = tim();
    }
    int g, v = SCORE_UNDEFINED;
    if (board.is_end()){
        g = board.score_player();
        cerr << "depth " << board.n_discs() - 4 << " LF value " << g << endl;
        book.reg(board, -g);
        return g;
    }
    if (board.n_discs() >= 4 + book_depth){
        g = book_learn_calc_value(board, level);
        cerr << "depth " << board.n_discs() - 4 << " LF value " << g << endl;
        book.reg(board, -g);
        return g;
    }
    if (get_level_complete_depth(level) >= HW2 - board.n_discs())
        expected_error = 0;
    uint64_t legal = board.get_legal();
    if (legal == 0ULL){
        board.pass();
        *player ^= 1;
            g = -book_learn_search(board, level, book_depth, error_sum, expected_error, adopt_error_sum, board_copy, player, strt_tim, book_file, book_bak);
        *player ^= 1;
        board.pass();
        return g;
    }
    Search_result best_move = ai(board, level, true, 0, false);
    cerr << "depth " << board.n_discs() - 5 << " BM value " << best_move.value << " move " << idx_to_coord(best_move.policy) << endl;
    Flip flip;
    bool alpha_updated = false;
    calc_flip(&flip, &board, (uint8_t)best_move.policy);
    board.move_board(&flip);
    *player ^= 1;
        board.copy(board_copy);
        g = -book_learn_search(board, level, book_depth, error_sum, expected_error, adopt_error_sum, board_copy, player, strt_tim, book_file, book_bak);
        if (global_searching){
            v = g;
            cerr << "depth " << board.n_discs() - 5 << " PV value " << g << " move " << idx_to_coord(best_move.policy) << " remaining error " << adopt_error_sum - error_sum << endl;
            //policies.emplace_back(best_move.policy);
        }
    *player ^= 1;
    board.undo_board(&flip);
    legal ^= 1ULL << best_move.policy;
    int n_error_sum, alpha;
    for (uint_fast8_t cell = first_bit(&legal); legal; cell = next_bit(&legal)){
        calc_flip(&flip, &board, cell);
        board.move_board(&flip);
        *player ^= 1;
            board.copy(board_copy);
            alpha = best_move.value - expected_error;
            g = -ai_nws(board, level, -alpha, -alpha + 1, true);
            //cerr << "PRESEARCH depth " << board.n_discs() - 5 << "    value " << g << " move " << idx_to_coord(cell) << " [" << alpha << "," << beta << "]" << endl;
            if (global_searching && g >= alpha && g <= HW2){
                n_error_sum = error_sum + min(0, best_move.value - g);
                g = -book_learn_search(board, level, book_depth, n_error_sum, expected_error, adopt_error_sum, board_copy, player, strt_tim, book_file, book_bak);
                if (global_searching){
                    v = max(v, g);
                    cerr << "depth " << board.n_discs() - 5 << " AD value " << g << " move " << idx_to_coord(cell) << " remaining error " << adopt_error_sum - n_error_sum << endl;
                }
            }
        *player ^= 1;
        board.undo_board(&flip);
    }
    if (global_searching){
        cerr << "depth " << board.n_discs() - 4 << " RG value " << v << endl;
        book.reg(board, -v);
    }
    return v;
}

inline void learn_book(Board root_board, int level, const int book_depth, int expected_error, Board *board_copy, int *player, string book_file, string book_bak, bool *book_learning){
    uint64_t strt_tim = tim();
    cerr << "book learn started" << endl;
    const int adopt_error_sum = max(expected_error, (book_depth + 4 - root_board.n_discs()) * expected_error / 5);
    int g = book_learn_search(root_board, level, book_depth, 0, expected_error, adopt_error_sum, board_copy, player, &strt_tim, book_file, book_bak);
    //if (*book_learning && global_searching)
    //    book.reg(root_board, -g);
    root_board.copy(board_copy);
    cerr << "book learn finished " << g << endl;
    *book_learning = false;
}
