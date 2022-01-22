﻿#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <vector>
#include <thread>
#include <future>
#include <chrono>
#include <fstream>
#include <sstream>
#include <time.h>
#include <queue>
#include "setting.hpp"
#include "common.hpp"
#include "board.hpp"
#include "book.hpp"
#include "evaluate.hpp"
#include "transpose_table.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "endsearch.hpp"
#include "book.hpp"
#include "human_value.hpp"
#include "joseki.hpp"
#include "umigame.hpp"
#if USE_MULTI_THREAD
	#include "thread_pool.hpp"
#endif
#include "gui/pulldown.hpp"
#include "gui/graph.hpp"
#include "gui/menu.hpp"

using namespace std;

constexpr Color font_color = Palette::Black;
constexpr int board_sx = 50, board_sy = 70, board_cell_size = 60, board_cell_frame_width = 1;
constexpr int stone_size = 25, legal_size = 5;
constexpr int graph_sx = 575, graph_sy = 245, graph_width = 415, graph_height = 345, graph_resolution = 10, graph_font_size = 15;

bool ai_init() {
	board_init();
	search_init();
	transpose_table_init();
	if (!evaluate_init())
		return false;
	if (!book_init())
		return false;
	if (!joseki_init())
		return false;
	if (!human_value_init())
		return false;
	return true;
}

Menu create_menu(bool *start_game_flag,
	bool *use_ai_flag, bool *human_first, bool *human_second, bool *both_ai,
	bool *use_hint_flag, bool *normal_hint, bool *human_hint, bool *umigame_hint,
	bool *use_value_flag,
	bool *start_book_learn_flag) {
	Menu menu;
	menu_title title;
	menu_elem menu_e, side_menu;
	Font menu_font(15);
	Texture checkbox(U"resources/img/checked.png", TextureDesc::Mipped);

	title.init(U"対局");

	menu_e.init_button(U"新規対局", start_game_flag);
	title.push(menu_e);

	menu.push(title);



	title.init(U"設定");

	menu_e.init_check(U"AIが着手", use_ai_flag, *use_ai_flag);
	side_menu.init_radio(U"人間先手", human_first, *human_first);
	menu_e.push(side_menu);
	side_menu.init_radio(U"人間後手", human_second, *human_second);
	menu_e.push(side_menu);
	side_menu.init_radio(U"AI同士", both_ai, *both_ai);
	menu_e.push(side_menu);
	title.push(menu_e);

	menu_e.init_check(U"ヒント表示", use_hint_flag, *use_hint_flag);
	side_menu.init_check(U"石差評価", normal_hint, *normal_hint);
	menu_e.push(side_menu);
	side_menu.init_check(U"人間的評価", human_hint, *human_hint);
	menu_e.push(side_menu);
	side_menu.init_check(U"うみがめ数", umigame_hint, *umigame_hint);
	menu_e.push(side_menu);
	title.push(menu_e);

	menu_e.init_check(U"評価値表示", use_value_flag, *use_value_flag);
	title.push(menu_e);

	menu.push(title);



	title.init(U"book");

	menu_e.init_button(U"学習開始", start_book_learn_flag);
	title.push(menu_e);

	menu.push(title);


	menu.init(0, 0, menu_font, checkbox);
	return menu;
}

pair<bool, board> move_board(board b, bool board_clicked[]) {
	board res;
	for (int cell = 0; cell < hw2; ++cell) {
		if (board_clicked[cell]) {
			res = b.move(cell);
			res.check_player();
			return make_pair(true, res);
		}
	}
	return make_pair(false, b);
}

search_result ai_return_result(search_result result) {
	return result;
}

search_result ai_book_return(board bd, book_value book_result) {
	search_result res;
	res.policy = book_result.policy;
	res.value = book_result.value;
	res.depth = -1;
	res.nps = 0;
	cerr << "book policy " << res.policy << " value " << res.value << endl;
	return res;
}

inline future<search_result> ai(board bd, int depth, int end_depth, int book_accept) {
	constexpr int first_moves[4] = { 19, 26, 37, 44 };
	int policy;
	search_result result;
	if (bd.n == 4) {
		policy = first_moves[myrandrange(0, 4)];
		result.policy = policy;
		result.value = 0;
		result.depth = 0;
		result.nps = 0;
		return async(launch::async, ai_return_result, result);
	}
	book_value book_result = book.get_random(&bd, book_accept);
	if (book_result.policy != -1) {
		return async(launch::async, ai_book_return, bd, book_result);
	}
	if (bd.n >= hw2 - end_depth) {
		return async(launch::async, endsearch, bd, tim(), false);
	}
	cerr << bd.p << endl;
	return async(launch::async, midsearch, bd, tim(), depth);
}

inline void create_vacant_lst(board bd) {
	int bd_arr[hw2];
	bd.translate_to_arr(bd_arr);
	vacant_lst.clear();
	for (int i = 0; i < hw2; ++i) {
		if (bd_arr[i] == vacant)
			vacant_lst.push_back(i);
	}
	if (bd.n < hw2_m1)
		sort(vacant_lst.begin(), vacant_lst.end(), cmp_vacant);
}

int find_history_idx(vector<board> history, int history_place) {
	for (int i = 0; i < (int)history.size(); ++i){
		if (history[i].n - 4 == history_place)
			return i;
	}
	return 0;
}

void initialize_draw(future<bool> *f, bool *initializing, bool *initialize_failed, Font font) {
	if (!(*initialize_failed)) {
		font(U"AI初期化中").draw(50, 50, font_color);
		if (f->wait_for(chrono::seconds(0)) == future_status::ready) {
			if (f->get()) {
				*initializing = false;
			}
			else {
				*initialize_failed = true;
			}
		}
	}
	else {
		font(U"AI初期化失敗").draw(50, 50, font_color);
	}
}

void board_draw(Rect board_cells[], board b) {
	for (int cell = 0; cell < hw2; ++cell) {
		board_cells[cell].draw(Palette::Green).drawFrame(board_cell_frame_width, 0, Palette::Black);
	}
	Circle(board_sx + 2 * board_cell_size, board_sy + 2 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 2 * board_cell_size, board_sy + 6 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 6 * board_cell_size, board_sy + 2 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 6 * board_cell_size, board_sy + 6 * board_cell_size, 5).draw(Palette::Black);
	int board_arr[hw2], y, x;
	b.translate_to_arr(board_arr);
	for (int cell = 0; cell < hw2; ++cell) {
		x = board_sx + (cell % hw) * board_cell_size + board_cell_size / 2;
		y = board_sy + (cell / hw) * board_cell_size + board_cell_size / 2;
		if (board_arr[cell] == black) {
			Circle(x, y, stone_size).draw(Palette::Black);
		}
		else if (board_arr[cell] == white) {
			Circle(x, y, stone_size).draw(Palette::White);
		}
		else if (b.legal(cell)) {
			Circle(x, y, legal_size).draw(Palette::Blue);
		}
	}
}

void Main() {
	Size window_size = Size(1000, 720);
	Window::Resize(window_size);
	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetTitle(U"Egaroucid5.2.0");
	//System::SetTerminationTriggers(UserAction::NoAction);
	Scene::SetBackground(Palette::White);
	Console.open();

	bool start_game_flag;
	bool use_ai_flag = true, human_first = true, human_second = false, both_ai = false;
	bool use_hint_flag = true, normal_hint = true, human_hint = true, umigame_hint = true;
	bool use_value_flag = true;
	bool start_book_learn_flag;
	Menu menu = create_menu(&start_game_flag,
		&use_ai_flag, &human_first, &human_second, &both_ai,
		&use_hint_flag, &normal_hint, &human_hint, &umigame_hint,
		&use_value_flag,
		&start_book_learn_flag);
	Rect board_cells[hw2];
	for (int cell = 0; cell < hw2; ++cell) {
		board_cells[cell] = Rect(board_sx + (cell % hw) * board_cell_size, board_sy + (cell / hw) * board_cell_size, board_cell_size, board_cell_size);
	}
	Font graph_font(graph_font_size);
	Graph graph;
	graph.sx = graph_sx;
	graph.sy = graph_sy;
	graph.size_x = graph_width;
	graph.size_y = graph_height;
	graph.resolution = graph_resolution;
	graph.font = graph_font;
	graph.font_size = graph_font_size;
	Font font50(50);

	board bd;
	vector<board> history, fork_history;
	int history_place = 0;
	bool board_clicked[hw2];
	bool fork_mode = false;

	future<bool> initialize_future = async(launch::async, ai_init);
	bool initializing = true, initialize_failed = false;

	future<search_result> ai_future;
	bool ai_thinking = false;
	int ai_value = 0;
	int ai_depth1 = 13, ai_depth2 = 20, ai_book_accept = 0;


	while (System::Update()) {
		if (initializing) {
			initialize_draw(&initialize_future,&initializing, &initialize_failed, font50);
			if (!initializing) {
				bd.reset();
				bd.v = -inf;
				history.emplace_back(bd);
				history_place = 0;
				fork_mode = false;
			}
		}
		else {
			for (int cell = 0; cell < hw2; ++cell) {
				board_clicked[cell] = board_cells[cell].leftClicked() && !menu.active() && bd.legal(cell);
			}
			if (start_game_flag) {
				cerr << "reset" << endl;
				bd.reset();
				bd.v = -inf;
				history.clear();
				history.emplace_back(bd);
				fork_history.clear();
				history_place = 0;
				fork_mode = false;
			}
			if (bd.p != vacant && (!use_ai_flag || (human_first && bd.p == black) || (human_second && bd.p == white) || fork_mode)) {
				pair<bool, board> moved_board = move_board(bd, board_clicked);
				if (moved_board.first) {
					bd = moved_board.second;
					bd.check_player();
					bd.v = -inf;
					if (fork_mode) {
						while (fork_history.size()) {
							if (fork_history[fork_history.size() - 1].n >= bd.n) {
								fork_history.pop_back();
							}
							else {
								break;
							}
						}
						fork_history.emplace_back(bd);
					}
					else {
						history.emplace_back(bd);
					}
					history_place = bd.n - 4;
				}
			}
			else if (bd.p != vacant && history[history.size() - 1].n - 4 == history_place) {
				if (ai_thinking) {
					if (ai_future.wait_for(chrono::seconds(0)) == future_status::ready) {
						search_result ai_result = ai_future.get();
						int sgn = (bd.p ? -1 : 1);
						bd = bd.move(ai_result.policy);
						bd.check_player();
						bd.v = sgn * ai_result.value;
						history.emplace_back(bd);
						history_place = bd.n - 4;
						ai_value = ai_result.value;
						ai_thinking = false;
					}
				}
				else {
					create_vacant_lst(bd);
					ai_future = ai(bd, ai_depth1, ai_depth2, ai_book_accept);
					ai_thinking = true;
				}
			}

			int former_history_place = history_place;
			history_place = graph.update_place(history, fork_history, history_place);
			if (history_place != former_history_place && !ai_thinking) {
				if (fork_mode && history_place > fork_history[fork_history.size() - 1].n - 4) {
					fork_history.clear();
					bd = history[find_history_idx(history, history_place)];
					if (history_place == history[history.size() - 1].n - 4) {
						fork_mode = false;
					}
					else {
						fork_history.emplace_back(bd);
					}
				}
				else if (!fork_mode || (fork_mode && history_place <= fork_history[0].n - 4)) {
					fork_mode = true;
					fork_history.clear();
					bd = history[find_history_idx(history, history_place)];
					fork_history.emplace_back(bd);
				}
				else if (fork_mode) {
					bd = fork_history[find_history_idx(fork_history, history_place)];
				}
			}

			board_draw(board_cells, bd);
			graph.draw(history, fork_history, history_place);
		}

		menu.draw();
	}

}
