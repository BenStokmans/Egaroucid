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
#include "level.hpp"
#include "mobility.hpp"
#include "board.hpp"
#include "book.hpp"
#include "evaluate.hpp"
#include "transpose_table.hpp"
#include "search.hpp"
#include "midsearch.hpp"
#include "endsearch.hpp"
#include "ai.hpp"
#include "human_value.hpp"
#include "joseki.hpp"
#include "umigame.hpp"
#include "thread_pool.hpp"
#include "gui/gui_common.hpp"
#include "gui/pulldown.hpp"
#include "gui/graph.hpp"
#include "gui/menu.hpp"
#include "gui/language.hpp"
#include "gui/button.hpp"

using namespace std;

#define search_final_define 100
#define search_book_define -1
#define hint_not_calculated_define 0

#define left_left 20
#define left_center 255
#define left_right 490
#define right_left 510
#define right_center 745
#define right_right 980
#define x_center 500
#define y_center 360

constexpr Color font_color = Palette::White;;
constexpr int board_size = 480, board_coord_size = 30;
constexpr int board_sx = left_left + board_coord_size, board_sy = y_center - board_size / 2, board_cell_size = board_size / hw, board_cell_frame_width = 2, board_frame_width = 7;
constexpr int stone_size = 25, legal_size = 5;
constexpr int graph_sx = 585, graph_sy = 245, graph_width = 400, graph_height = 345, graph_resolution = 10, graph_font_size = 15;
constexpr Color green = Color(36, 153, 114, 100);
constexpr int start_game_how_to_use_width = 200, start_game_how_to_use_height = 50;
constexpr int start_game_button_x = right_center - start_game_how_to_use_width / 2,
	start_game_button_y = y_center - start_game_how_to_use_height - 10,
	start_game_button_w = start_game_how_to_use_width,
	start_game_button_h = start_game_how_to_use_height,
	start_game_button_r = 10;
constexpr int how_to_use_button_x = right_center - start_game_how_to_use_width / 2,
	how_to_use_button_y = y_center + start_game_how_to_use_height + 10,
	how_to_use_button_w = start_game_how_to_use_width,
	how_to_use_button_h = start_game_how_to_use_height,
	how_to_use_button_r = 10;
constexpr Color button_color = Palette::White, button_font_color = Palette::Black;
constexpr int popup_width = 500, popup_height = 300, popup_r = 20, popup_circle_r = 30;
constexpr Color popup_color = Palette::White, popup_font_color = Palette::Black;

struct cell_value {
	int value;
	int depth;
};

bool ai_init() {
	mobility_init();
	board_init();
	transpose_table_init();
	if (!evaluate_init())
		return false;
	if (!book_init())
		return false;
	if (!joseki_init())
		return false;
	//if (!human_value_init())
	//	return false;
	thread_pool.resize(16);
	return true;
}

bool lang_initialize(string file) {
	return language.init(file);
}

Menu create_menu(Texture checkbox,
	bool *dammy,
	bool *entry_mode, bool *professional_mode, bool *serious_game,
	bool *start_game_flag, bool *analyze_flag,
	bool *use_ai_flag, bool *human_first, bool *human_second, bool *both_ai,
	bool *use_hint_flag, bool *normal_hint, bool *human_hint, bool *umigame_hint,
	bool *use_value_flag,
	int *ai_level, int *hint_level, int *book_error,
	bool *start_book_learn_flag,
	bool *output_record_flag, bool *output_game_flag, bool *input_record_flag, bool *input_board_flag,
	bool *show_end_popup,
	bool *thread1, bool* thread2, bool* thread4, bool* thread8, bool* thread16, bool* thread32, bool* thread64, bool* thread128,
	bool lang_acts[], vector<string> lang_name_vector) {
	Menu menu;
	menu_title title;
	menu_elem menu_e, side_menu;
	Font menu_font(15);

	title.init(language.get("mode", "mode"));

	menu_e.init_radio(language.get("mode", "entry_mode"), entry_mode, *entry_mode);
	title.push(menu_e);
	menu_e.init_radio(language.get("mode", "professional_mode"), professional_mode, *professional_mode);
	title.push(menu_e);
	menu_e.init_radio(language.get("mode", "serious_game"), serious_game, *serious_game);
	title.push(menu_e);

	menu.push(title);

	title.init(language.get("play", "game"));

	menu_e.init_button(language.get("play", "new_game"), start_game_flag);
	title.push(menu_e);
	menu_e.init_button(language.get("play", "analyze"), analyze_flag);
	title.push(menu_e);

	menu.push(title);

	title.init(language.get("settings", "settings"));

	menu_e.init_check(language.get("settings", "ai_play", "ai_play"), use_ai_flag, *use_ai_flag);
	side_menu.init_radio(language.get("settings", "ai_play", "human_first"), human_first, *human_first);
	menu_e.push(side_menu);
	side_menu.init_radio(language.get("settings", "ai_play", "human_second"), human_second, *human_second);
	menu_e.push(side_menu);
	side_menu.init_radio(language.get("settings", "ai_play", "both_ai"), both_ai, *both_ai);
	menu_e.push(side_menu);
	title.push(menu_e);

	if (!(*serious_game)) {
		menu_e.init_check(language.get("settings", "hint", "hint"), use_hint_flag, *use_hint_flag);
		if (*professional_mode) {
			side_menu.init_check(language.get("settings", "hint", "stone_value"), normal_hint, *normal_hint);
			menu_e.push(side_menu);
			side_menu.init_check(language.get("settings", "hint", "human_value"), human_hint, *human_hint);
			menu_e.push(side_menu);
			side_menu.init_check(language.get("settings", "hint", "umigame_value"), umigame_hint, *umigame_hint);
			menu_e.push(side_menu);
		}
		title.push(menu_e);
	}

	menu_e.init_check(language.get("settings", "graph"), use_value_flag, *use_value_flag);
	title.push(menu_e);

	if (!(*entry_mode)) {
		menu_e.init_button(language.get("settings", "thread", "thread"), dammy);
		side_menu.init_radio(language.get("settings", "thread", "1"), thread1, *thread1);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "2"), thread2, *thread2);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "4"), thread4, *thread4);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "8"), thread8, *thread8);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "16"), thread16, *thread16);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "32"), thread32, *thread32);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "64"), thread64, *thread64);
		menu_e.push(side_menu);
		side_menu.init_radio(language.get("settings", "thread", "128"), thread128, *thread128);
		menu_e.push(side_menu);
		title.push(menu_e);
	}

	if (*entry_mode) {
		menu_e.init_button(language.get("level", "level"), dammy);
		side_menu.init_bar(language.get("level", "ai_level"), ai_level, *ai_level);
	}
	else if (*professional_mode) {

	}
	else if (*serious_game) {
		*book_error = 0;
	}

	menu.push(title);


	if (*professional_mode) {
		title.init(language.get("book", "book"));

		menu_e.init_button(language.get("book", "learn"), start_book_learn_flag);
		title.push(menu_e);

		menu.push(title);



		title.init(language.get("in_out", "in_out"));

		menu_e.init_button(language.get("in_out", "output_record"), output_record_flag);
		title.push(menu_e);
		menu_e.init_button(language.get("in_out", "output_game"), output_game_flag);
		title.push(menu_e);
		menu_e.init_button(language.get("in_out", "input_record"), input_record_flag);
		title.push(menu_e);
		menu_e.init_button(language.get("in_out", "input_board"), input_board_flag);
		title.push(menu_e);

		menu.push(title);
	}

	title.init(language.get("display", "display"));

	menu_e.init_check(language.get("display", "end_popup"), show_end_popup, *show_end_popup);
	title.push(menu_e);

	menu.push(title);

	title.init(U"Language");
	for (int i = 0; i < (int)lang_name_vector.size(); ++i) {
		menu_e.init_radio(language_name.get(lang_name_vector[i]), &lang_acts[i], lang_acts[i]);
		title.push(menu_e);
	}
	menu.push(title);

	menu.init(0, 0, menu_font, checkbox);
	return menu;
}

pair<bool, board> move_board(board b, bool board_clicked[]) {
	mobility mob;
	for (int cell = 0; cell < hw2; ++cell) {
		if (board_clicked[cell]) {
			calc_flip(&mob, &b, cell);
			b.move(&mob);
			b.check_player();
			return make_pair(true, b);
		}
	}
	return make_pair(false, b);
}

cell_value hint_search(board b, int level, int policy) {
	cell_value res;
	mobility mob;
	int depth, end_depth;
	bool use_mpc;
	double mpct;
	calc_flip(&mob, &b, policy);
	b.move(&mob);
	get_level(level, b.n - 4, &depth, &end_depth, &use_mpc, &mpct);
	res.value = book.get(&b);
	if (res.value != -inf) {
		res.depth = search_book_define;
	}
	else if (hw2 - b.n <= end_depth) {
		res.value = -endsearch_value_nomemo(b, tim(), use_mpc, mpct).value;
		res.depth = use_mpc ? hw2 - b.n : search_final_define;
	}
	else {
		res.value = -midsearch_value_nomemo(b, tim(), depth, use_mpc, mpct).value;
		res.depth = depth;
	}
	return res;
}

cell_value analyze_search(board b, int level) {
	cell_value res;
	int depth, end_depth;
	bool use_mpc;
	double mpct;
	get_level(level, b.n - 4, &depth, &end_depth, &use_mpc, &mpct);
	res.value = book.get(&b) * (b.p ? 1 : -1);
	if (abs(res.value) != inf) {
		res.depth = search_book_define;
	}
	else if (hw2 - b.n <= end_depth) {
		res.value = endsearch_value_memo(b, tim(), use_mpc, mpct).value * (b.p ? -1 : 1);
		res.depth = use_mpc ? hw2 - b.n : search_final_define;
	}
	else {
		res.value = midsearch_value_memo(b, tim(), depth, use_mpc, mpct).value * (b.p ? -1 : 1);
		res.depth = depth;
	}
	return res;
}

inline void create_vacant_lst(board bd) {
	int bd_arr[hw2];
	bd.translate_to_arr(bd_arr);
	vacant_lst.clear();
	for (int i = 0; i < hw2; ++i) {
		if (bd_arr[i] == vacant)
			vacant_lst.push_back(hw2_m1 - i);
	}
	if (bd.n < hw2_m1)
		sort(vacant_lst.begin(), vacant_lst.end(), cmp_vacant);
}

int find_history_idx(vector<history_elem> history, int history_place) {
	for (int i = 0; i < (int)history.size(); ++i){
		if (history[i].b.n - 4 == history_place)
			return i;
	}
	return 0;
}

void initialize_draw(future<bool> *f, bool *initializing, bool *initialize_failed, Font font, Font small_font, Texture icon, Texture logo, bool texture_loaded) {
	icon.scaled((double)(left_right - left_left) / icon.width()).draw(left_left, y_center - (left_right - left_left) / 2);
	logo.scaled((double)(left_right - left_left) * 0.8 / logo.width()).draw(right_left, y_center - 30);
	if (!(*initialize_failed) && texture_loaded) {
		font(language.get("loading", "loading")).draw(right_left, y_center + font.fontSize(), font_color);
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
		small_font(language.get("loading", "load_failed")).draw(right_left, y_center + font.fontSize(), font_color);
	}
}

void lang_initialize_failed_draw(Font font, Font small_font, Texture icon, Texture logo) {
	icon.scaled((double)(left_right - left_left) / icon.width()).draw(left_left, y_center - (left_right - left_left) / 2);
	logo.scaled((double)(left_right - left_left) * 0.8 / logo.width()).draw(right_left, y_center - 30);
	small_font(U"言語パックを読み込めませんでした\nresourcesフォルダを確認してください").draw(right_left, y_center + font.fontSize() * 3 / 2, font_color);
	small_font(U"Failed to load language pack\nPlease check the resources directory").draw(right_left, y_center + font.fontSize() * 3, font_color);
}

void closing_draw(Font font, Font small_font, Texture icon, Texture logo, bool texture_loaded) {
	icon.scaled((double)(left_right - left_left) / icon.width()).draw(left_left, y_center - (left_right - left_left) / 2);
	logo.scaled((double)(left_right - left_left) * 0.8 / logo.width()).draw(right_left, y_center - 30);
	font(language.get("closing")).draw(right_left, y_center + font.fontSize(), font_color);
}

void board_draw(Rect board_cells[], board b, int int_mode, bool use_hint_flag, bool normal_hint, bool human_hint, bool umigame_hint,
	const int hint_state[], const int hint_value[], const int hint_depth[], Font normal_font, Font small_font, Font big_font, Font mini_font, Font coord_font, 
	bool before_start_game,
	const int umigame_state[], const umigame_result umigame_value[],
	const int human_value_state, const int human_value[]) {
	String coord_x = U"abcdefgh";
	for (int i = 0; i < hw; ++i) {
		coord_font(i + 1).draw(Arg::center(board_sx - board_coord_size, board_sy + board_cell_size * i + board_cell_size / 2), font_color);
		coord_font(coord_x[i]).draw(Arg::center(board_sx + board_cell_size * i + board_cell_size / 2, board_sy - board_coord_size), font_color);
	}
	for (int i = 0; i < hw_m1; ++i) {
		Line(board_sx + board_cell_size * (i + 1), board_sy, board_sx + board_cell_size * (i + 1), board_sy + board_cell_size * hw).draw(board_cell_frame_width, Palette::Black);
		Line(board_sx, board_sy + board_cell_size * (i + 1), board_sx + board_cell_size * hw, board_sy + board_cell_size * (i + 1)).draw(board_cell_frame_width, Palette::Black);
	}
	Circle(board_sx + 2 * board_cell_size, board_sy + 2 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 2 * board_cell_size, board_sy + 6 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 6 * board_cell_size, board_sy + 2 * board_cell_size, 5).draw(Palette::Black);
	Circle(board_sx + 6 * board_cell_size, board_sy + 6 * board_cell_size, 5).draw(Palette::Black);
	RoundRect(board_sx, board_sy, board_cell_size * hw, board_cell_size * hw, 20).draw(green).drawFrame(0, board_frame_width, Palette::White);
	int board_arr[hw2], max_cell_value = -inf;
	mobility mob;
	unsigned long long legal = b.mobility_ull();
	b.translate_to_arr(board_arr);
	for (int cell = 0; cell < hw2; ++cell) {
		int x = board_sx + (cell % hw) * board_cell_size + board_cell_size / 2;
		int y = board_sy + (cell / hw) * board_cell_size + board_cell_size / 2;
		if (board_arr[cell] == black) {
			Circle(x, y, stone_size).draw(Palette::Black);
		}
		else if (board_arr[cell] == white) {
			Circle(x, y, stone_size).draw(Palette::White);
		}
		if (1 & (legal >> cell)) {
			if (use_hint_flag && normal_hint && hint_state[cell] >= 2) {
				max_cell_value = max(max_cell_value, hint_value[cell]);
			}
			if (!before_start_game && (!use_hint_flag || (!normal_hint && !human_hint && !umigame_hint))) {
				int xx = board_sx + (hw_m1 - cell % hw) * board_cell_size + board_cell_size / 2;
				int yy = board_sy + (hw_m1 - cell / hw) * board_cell_size + board_cell_size / 2;
				Circle(xx, yy, legal_size).draw(Palette::Cyan);
			}
		}
	}
	if (b.policy != -1) {
		Circle(board_sx + (hw_m1 - b.policy % hw) * board_cell_size + board_cell_size / 2, board_sy + (hw_m1 - b.policy / hw) * board_cell_size + board_cell_size / 2, legal_size).draw(Palette::Red);
	}
	if (use_hint_flag && b.p != vacant && !before_start_game) {
		bool hint_shown[hw2];
		for (int i = 0; i < hw2; ++i) {
			hint_shown[i] = false;
		}
		if (normal_hint) {
			for (int cell = 0; cell < hw2; ++cell) {
				if (1 & (legal >> cell)) {
					if (hint_state[cell] >= 2) {
						Color color = Palette::White;
						if (hint_value[cell] == max_cell_value)
							color = Palette::Cyan;
						if (int_mode == 0) {
							int x = board_sx + (hw_m1 - cell % hw) * board_cell_size + board_cell_size / 2;
							int y = board_sy + (hw_m1 - cell / hw) * board_cell_size + board_cell_size / 2;
							big_font(hint_value[cell]).draw(Arg::center = Vec2{ x, y }, color);
						}
						else if (int_mode == 1) {
							int x = board_sx + (hw_m1 - cell % hw) * board_cell_size + 3;
							int y = board_sy + (hw_m1 - cell / hw) * board_cell_size + 3;
							normal_font(hint_value[cell]).draw(x, y, color);
							y += 19;
							if (hint_depth[cell] == search_book_define) {
								small_font(U"book").draw(x, y, color);
							}
							else if (hint_depth[cell] == search_final_define) {
								small_font(U"100%").draw(x, y, color);
							}
							else {
								small_font(language.get("common", "level"), hint_depth[cell]).draw(x, y, color);
							}
						}
						hint_shown[cell] = true;
					}
					else {
						int x = board_sx + (hw_m1 - cell % hw) * board_cell_size + board_cell_size / 2;
						int y = board_sy + (hw_m1 - cell / hw) * board_cell_size + board_cell_size / 2;
						Circle(x, y, legal_size).draw(Palette::Cyan);
					}
				}
			}
		}
		if (umigame_hint && int_mode == 1) {
			for (int cell = 0; cell < hw2; ++cell) {
				if (1 & (legal >> cell)) {
					if (umigame_state[cell] == 2) {
						int umigame_sx = board_sx + (hw_m1 - cell % hw) * board_cell_size + 2;
						int umigame_sy = board_sy + (hw_m1 - cell / hw) * board_cell_size + 38;
						RectF black_rect = mini_font(umigame_value[cell].b).region(umigame_sx, umigame_sy);
						mini_font(umigame_value[cell].b).draw(umigame_sx, umigame_sy, Palette::Black);
						umigame_sx += black_rect.size.x;
						mini_font(umigame_value[cell].w).draw(umigame_sx, umigame_sy, Palette::White);
					}
				}
			}
		}
		if (human_hint && int_mode == 1) {
			if (human_value_state == 2) {
				int max_human_value = -inf;
				for (int cell = 0; cell < hw2; ++cell) {
					if (1 & (legal >> cell)) {
						max_human_value = max(max_human_value, human_value[cell]);
					}
				}
				for (int cell = 0; cell < hw2; ++cell) {
					if (1 & (legal >> cell)) {
						Color color = Palette::White;
						if (human_value[cell] == max_human_value)
							color = Palette::Cyan;
						int x = board_sx + (hw_m1 - cell % hw + 1) * board_cell_size - 3;
						int y = board_sy + (hw_m1 - cell / hw) * board_cell_size + 3;
						mini_font(human_value[cell]).draw(Arg::topRight(x, y), color);
					}
				}
			}
		}
	}
}

bool show_popup(board b, bool use_ai_flag, bool human_first, bool human_second, bool both_ai, Font big_font, Font small_font) {
	RoundRect(x_center - popup_width / 2, y_center - popup_height / 2, popup_width, popup_height, popup_r).draw(popup_color);
	int black_stones = pop_count_ull(b.b);
	int white_stones = pop_count_ull(b.w);
	if (use_ai_flag && human_first) {
		if (black_stones > white_stones) {
			big_font(language.get("result", "you_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else if (black_stones < white_stones) {
			big_font(language.get("result", "AI_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else {
			big_font(language.get("result", "draw")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
	}
	else if (use_ai_flag && human_second) {
		if (black_stones < white_stones) {
			big_font(language.get("result", "you_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else if (black_stones > white_stones) {
			big_font(language.get("result", "AI_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else {
			big_font(language.get("result", "draw")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
	}
	else {
		if (black_stones > white_stones) {
			big_font(language.get("result", "black_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else if (black_stones < white_stones) {
			big_font(language.get("result", "white_win")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
		else {
			big_font(language.get("result", "draw")).draw(Arg::bottomCenter(x_center, y_center - 60), popup_font_color);
		}
	}
	Circle(x_center - popup_width / 3, y_center, popup_circle_r).draw(Palette::Black);
	small_font(black_stones).draw(Arg::rightCenter(x_center - popup_width / 3 + popup_circle_r * 2 + 20, y_center), popup_font_color);
	Circle(x_center + popup_width / 3, y_center, popup_circle_r).draw(Palette::White).drawFrame(2, Palette::Black);
	small_font(white_stones).draw(Arg::leftCenter(x_center + popup_width / 3 - popup_circle_r * 2 - 20, y_center), popup_font_color);
	FrameButton button;
	button.init(x_center - 100, y_center + 60, 200, 50, 10, 2, language.get("button", "close"), small_font, button_color, button_font_color, button_font_color);
	button.draw();
	return !button.clicked();
}

void reset_hint(int hint_state[], future<cell_value> hint_future[]) {
	global_searching = false;
	for (int i = 0; i < hw2; ++i) {
		if (hint_state[i] % 2 == 1) {
			hint_future[i].get();
		}
		hint_state[i] = hint_not_calculated_define;
	}
	global_searching = true;
}

void reset_umigame(int umigame_state[], future<umigame_result> umigame_future[]) {
	global_searching = false;
	for (int i = 0; i < hw2; ++i) {
		if (umigame_state[i] == 1) {
			umigame_future[i].get();
		}
		umigame_state[i] = hint_not_calculated_define;
	}
	global_searching = true;
}

void reset_human_value(int *human_value_state, future<void>* human_value_future) {
	global_searching = false;
	if (*human_value_state == 1) {
		human_value_future->get();
	}
	*human_value_state = 0;
	global_searching = true;
}

void reset_ai(bool *ai_thinking, future<search_result> *ai_future) {
	if (*ai_thinking) {
		global_searching = false;
		ai_future->get();
		global_searching = true;
		*ai_thinking = false;
	}
}

bool not_finished(board bd) {
	return bd.p == 0 || bd.p == 1;
}

umigame_result get_umigame_p(board b) {
	return umigame.get(&b);
}

future<umigame_result> get_umigame(board b) {
	return async(launch::async, get_umigame_p, b);
}

future<void> get_human_value(board b, int depth, double a, int res[]) {
	return async(launch::async, calc_all_human_value, b, depth, a, res);
}

int import_int(ifstream* ifs) {
	string line;
	if (!getline(*ifs, line)) {
		cerr << "setting NOT imported" << endl;
		return -inf;
	}
	try {
		return stoi(line);
	}
	catch (const invalid_argument& ex) {
		cerr << "setting NOT imported" << endl;
		return -inf;
	}
}

string import_str(ifstream* ifs) {
	string line;
	if (!getline(*ifs, line)) {
		cerr << "setting NOT imported" << endl;
		return "undefined";
	}
	return line;
}

bool import_setting(int *int_mode, int *ai_level, int *ai_book_accept, int *hint_level,
	bool *use_ai_flag, int *use_ai_mode,
	bool *use_hint_flag, bool *normal_hint, bool *human_hint, bool *umigame_hint,
	bool *show_end_popup,
	int *n_thread_idx,
	string *lang_name) {
	ifstream ifs("resources/settings.txt");
	if (ifs.fail()) {
		return false;
	}
	*int_mode = import_int(&ifs);
	if (*int_mode == -inf) {
		return false;
	}
	*ai_level = import_int(&ifs);
	if (*ai_level == -inf) {
		return false;
	}
	*ai_book_accept = import_int(&ifs);
	if (*ai_book_accept == -inf) {
		return false;
	}
	*hint_level = import_int(&ifs);
	if (*hint_level == -inf) {
		return false;
	}
	*use_ai_flag = import_int(&ifs);
	if (*use_ai_flag == -inf) {
		return false;
	}
	*use_ai_mode = import_int(&ifs);
	if (*use_ai_mode == -inf) {
		return false;
	}
	*use_hint_flag = import_int(&ifs);
	if (*use_hint_flag == -inf) {
		return false;
	}
	*normal_hint = import_int(&ifs);
	if (*normal_hint == -inf) {
		return false;
	}
	*human_hint = import_int(&ifs);
	if (*human_hint == -inf) {
		return false;
	}
	*umigame_hint = import_int(&ifs);
	if (*umigame_hint == -inf) {
		return false;
	}
	*show_end_popup = import_int(&ifs);
	if (*show_end_popup == -inf) {
		return false;
	}
	*n_thread_idx = import_int(&ifs);
	if (*n_thread_idx == -inf) {
		return false;
	}
	*lang_name = import_str(&ifs);
	if (*lang_name == "undefined") {
		return false;
	}
	return true;
}

void export_setting(int int_mode, int ai_level, int ai_book_accept, int hint_level,
	bool use_ai_flag, int use_ai_mode,
	bool use_hint_flag, bool normal_hint, bool human_hint, bool umigame_hint,
	bool show_end_popup,
	int n_thread_idx,
	string lang_name) {
	ofstream ofs("resources/settings.txt");
	if (!ofs.fail()) {
		ofs << int_mode << endl;
		ofs << ai_level << endl;
		ofs << ai_book_accept << endl;
		ofs << hint_level << endl;
		ofs << use_ai_flag << endl;
		ofs << use_ai_mode << endl;
		ofs << use_hint_flag << endl;
		ofs << normal_hint << endl;
		ofs << human_hint << endl;
		ofs << umigame_hint << endl;
		ofs << show_end_popup << endl;
		ofs << n_thread_idx << endl;
		ofs << lang_name << endl;
	}
}

String str_record(int policy) {
	return Unicode::Widen(string({ (char)('a' + hw_m1 - policy % hw), (char)('1' + hw_m1 - policy / hw) }));
}

bool import_record(String record, vector<history_elem>* n_history) {
	board h_bd;
	bool flag = true;
	String record_tmp = U"";
	if (record.size() % 2 != 0) {
		flag = false;
	}
	else {
		int y, x;
		unsigned long long legal;
		mobility mob;
		h_bd.reset();
		h_bd.v = -inf;
		n_history->emplace_back(history_elem(h_bd, U""));
		for (int i = 0; i < record.size(); i += 2) {
			x = (int)record[i] - (int)'a';
			if (x < 0 || hw <= x) {
				x = (int)record[i] - (int)'A';
				if (x < 0 || hw <= x) {
					flag = false;
					break;
				}
			}
			y = (int)record[i + 1] - (int)'1';
			if (y < 0 || hw <= y) {
				flag = false;
				break;
			}
			y = hw_m1 - y;
			x = hw_m1 - x;
			legal = h_bd.mobility_ull();
			if (1 & (legal >> (y * hw + x))) {
				calc_flip(&mob, &h_bd, y * hw + x);
				h_bd.move(&mob);
				h_bd.check_player();
				if (h_bd.p == vacant) {
					if (i != record.size() - 1) {
						flag = false;
						break;
					}
				}
			}
			else {
				flag = false;
				break;
			}
			h_bd.v = -inf;
			n_history->emplace_back(history_elem(h_bd, n_history->at(n_history->size() - 1).record + str_record(y * hw + x)));
		}
	}
	return flag;
}

pair<bool, board> import_board(String board_str) {
	bool flag = true;
	int player = -1;
	int bd_arr[hw2];
	board bd;
	if (board_str.size() != hw2 + 1) {
		flag = false;
	}
	else {
		for (int i = 0; i < hw2; ++i) {
			if (board_str[i] == '0' || board_str[i] == 'B' || board_str[i] == 'b' || board_str[i] == 'X' || board_str[i] == 'x' || board_str[i] == '*')
				bd_arr[i] = black;
			else if (board_str[i] == '1' || board_str[i] == 'W' || board_str[i] == 'w' || board_str[i] == 'O' || board_str[i] == 'o')
				bd_arr[i] = white;
			else if (board_str[i] == '.' || board_str[i] == '-')
				bd_arr[i] = vacant;
			else {
				flag = false;
				break;
			}
		}
		if (board_str[hw2] == '0' || board_str[hw2] == 'B' || board_str[hw2] == 'b' || board_str[hw2] == 'X' || board_str[hw2] == 'x' || board_str[hw2] == '*')
			player = 0;
		else if (board_str[hw2] == '1' || board_str[hw2] == 'W' || board_str[hw2] == 'w' || board_str[hw2] == 'O' || board_str[hw2] == 'o')
			player = 1;
		else
			flag = false;
	}
	if (flag) {
		bd.translate_from_arr(bd_arr, player);
	}
	return make_pair(flag, bd);
}

bool close_app(int hint_state[], future<cell_value> hint_future[],
	int umigame_state[], future<umigame_result> umigame_future[],
	int* human_value_state, future<void>* human_value_future,
	bool* ai_thinking, future<search_result>* ai_future,
	int int_mode, int ai_level, int ai_book_accept, int hint_level,
	bool use_ai_flag, int use_ai_mode,
	bool use_hint_flag, bool normal_hint, bool human_hint, bool umigame_hint,
	bool show_end_popup,
	int n_thread_idx,
	string lang_name) {
	reset_hint(hint_state, hint_future);
	reset_umigame(umigame_state, umigame_future);
	reset_human_value(human_value_state, human_value_future);
	reset_ai(ai_thinking, ai_future);
	export_setting(int_mode, ai_level, ai_book_accept, hint_level,
		use_ai_flag, use_ai_mode,
		use_hint_flag, normal_hint, human_hint, umigame_hint,
		show_end_popup,
		n_thread_idx,
		lang_name);
	return true;
}

void Main() {
	Size window_size = Size(1000, 720);
	Window::Resize(window_size);
	Window::SetStyle(WindowStyle::Sizable);
	Scene::SetResizeMode(ResizeMode::Keep);
	Window::SetTitle(U"Egaroucid5.3.0");
	System::SetTerminationTriggers(UserAction::NoAction);
	Scene::SetBackground(green);
	Console.open();

	bool dammy;
	constexpr int mode_size = 3;
	bool show_mode[mode_size] = {true, false, false};
	int int_mode = 0;
	bool start_game_flag = false, analyze_flag = false;
	bool use_ai_flag = true, human_first = true, human_second = false, both_ai = false;
	bool use_hint_flag = true, normal_hint = true, human_hint = true, umigame_hint = true;
	bool use_value_flag = true;
	bool start_book_learn_flag = false;
	bool output_record_flag = false, output_game_flag = false, input_record_flag = false, input_board_flag = false;
	bool texture_loaded = true;
	bool n_threads[8] = {false, false, true, false, false, false, false, false};
	int n_threads_num[8] = {1, 2, 4, 8, 16, 32, 64, 128};
	int n_thread_idx = 2;
	bool language_acts[100];
	language_acts[0] = true;
	for (int i = 1; i < 100; ++i) {
		language_acts[i] = false;
	}
	vector<string> language_names;
	ifstream ifs("resources/languages/languages.txt");
	string lang_line;
	while (getline(ifs, lang_line)) {
		language_names.emplace_back(lang_line);
	}
	Texture icon(U"resources/img/icon.png", TextureDesc::Mipped);
	Texture logo(U"resources/img/logo.png", TextureDesc::Mipped);
	Texture checkbox(U"resources/img/checked.png", TextureDesc::Mipped);
	if (icon.isEmpty() || logo.isEmpty() || checkbox.isEmpty()) {
		texture_loaded = false;
	}
	Rect board_cells[hw2];
	for (int cell = 0; cell < hw2; ++cell) {
		board_cells[cell] = Rect(board_sx + (hw_m1 - cell % hw) * board_cell_size, board_sy + (hw_m1 - cell / hw) * board_cell_size, board_cell_size, board_cell_size);
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
	Font board_coord_font(board_coord_size);
	Font font50(50);
	Font font30(30);
	Font font20(20);
	Font font15(15);
	Font normal_hint_font(18, Typeface::Bold);
	Font mini_hint_font(14, Typeface::Heavy);
	Font small_hint_font(10, Typeface::Bold);

	board bd;
	bool board_clicked[hw2];
	vector<history_elem> history, fork_history;
	int history_place = 0;
	bool fork_mode = false;

	int hint_value[hw2], hint_state[hw2], hint_depth[hw2];
	future<cell_value> hint_future[hw2];
	for (int i = 0; i < hw2; ++i) {
		hint_state[i] = 0;
	}

	int umigame_state[hw2];
	umigame_result umigame_value[hw2];
	future<umigame_result> umigame_future[hw2];
	for (int i = 0; i < hw2; ++i) {
		umigame_state[i] = 0;
	}

	int human_value_state = 0;
	int human_value[hw2];
	double human_value_a = 0.5;
	int human_value_depth = 5;
	future<void> human_value_future;

	future<bool> initialize_future = async(launch::async, ai_init);
	bool initializing = true, initialize_failed = false;

	Menu menu;

	future<search_result> ai_future;
	bool ai_thinking = false;
	int ai_value = 0;
	int ai_level = 21, ai_book_accept = 4, hint_level = 9;

	bool before_start_game = true;
	Button start_game_button;
	Button how_to_use_button;

	bool show_popup_flag = true;
	int showing_popup = 0;
	bool show_end_popup = true;

	bool analyzing = false;
	int analyze_idx = 0;
	future<cell_value> analyze_future;
	bool analyze_state = false;

	int starting_game = 0;

	bool closing = false;
	future<bool> closing_future;

	int use_ai_mode;
	string lang_name;
	if (!import_setting(&int_mode, &ai_level, &ai_book_accept, &hint_level,
		&use_ai_flag, &use_ai_mode,
		&use_hint_flag, &normal_hint, &human_hint, &umigame_hint,
		&show_end_popup,
		&n_thread_idx,
		&lang_name)) {
		cerr << "use default setting" << endl;
		int_mode = 0;
		ai_level = 15;
		ai_book_accept = 2;
		hint_level = 9;
		use_ai_flag = true;
		use_ai_mode = 0;
		use_hint_flag = true;
		normal_hint = true;
		human_hint = false;
		umigame_hint = false;
		show_end_popup = true;
		n_thread_idx = 2;
		lang_name = language_names[0];
	}
	for (int i = 0; i < mode_size; ++i) {
		show_mode[i] = i == int_mode;
	}
	for (int i = 0; i < 8; ++i) {
		n_threads[i] = i == n_thread_idx;
	}
	for (int i = 0; i < (int)language_names.size(); ++i) {
		language_acts[i] = lang_name == language_names[i];
	}

	int lang_initialized = 0;
	string lang_file = "resources/languages/" + lang_name + ".json";
	future<bool> lang_initialize_future = async(launch::async, lang_initialize, lang_file);
	language_name.init();


	while (System::Update()) {
		/*** terminate ***/
		if (System::GetUserActions() & UserAction::CloseButtonClicked) {
			closing = true;
			use_ai_mode = 0;
			if (human_first) {
				use_ai_mode = 0;
			}
			else if (human_second) {
				use_ai_mode = 1;
			}
			else if (both_ai) {
				use_ai_mode = 2;
			}
			closing_future = async(launch::async, close_app, hint_state, hint_future,
				umigame_state, umigame_future,
				&human_value_state, &human_value_future,
				&ai_thinking, &ai_future,
				int_mode, ai_level, ai_book_accept, hint_level,
				use_ai_flag, use_ai_mode,
				use_hint_flag, normal_hint, human_hint, umigame_hint,
				show_end_popup,
				n_thread_idx,
				lang_name);
		}
		if (closing) {
			closing_draw(font50, font20, icon, logo, texture_loaded);
			if (closing_future.wait_for(chrono::seconds(0)) == future_status::ready){
				closing_future.get();
				System::Exit();
			}
			continue;
		}
		/*** terminate ***/

		/*** initialize ***/
		if (initializing) {
			if (lang_initialized == 0) {
				if (lang_initialize_future.wait_for(chrono::seconds(0)) == future_status::ready) {
					if (lang_initialize_future.get()) {
						lang_initialized = 1;
					}
					else {
						lang_initialized = 3;
					}
				}
			}
			else if (lang_initialized == 1) {
				menu = create_menu(checkbox, &dammy,
					&show_mode[0], &show_mode[1], &show_mode[2],
					&start_game_flag,&analyze_flag,
					&use_ai_flag, &human_first, &human_second, &both_ai,
					&use_hint_flag, &normal_hint, &human_hint, &umigame_hint,
					&use_value_flag,
					&ai_level, &hint_level, &ai_book_accept,
					&start_book_learn_flag,
					&output_record_flag, &output_game_flag, &input_record_flag, &input_board_flag,
					&show_end_popup,
					&n_threads[0], &n_threads[1], &n_threads[2], &n_threads[3], &n_threads[4], &n_threads[5], &n_threads[6], &n_threads[7],
					language_acts, language_names);
				start_game_button.init(start_game_button_x, start_game_button_y, start_game_button_w, start_game_button_h, start_game_button_r, language.get("button", "start_game"), font30, button_color, button_font_color);
				how_to_use_button.init(how_to_use_button_x, how_to_use_button_y, how_to_use_button_w, how_to_use_button_h, how_to_use_button_r, language.get("button", "how_to_use"), font30, button_color, button_font_color);
				lang_initialized = 2;
			}
			else if (lang_initialized == 2) {
				if (!show_mode[int_mode]) {
					for (int i = 0; i < mode_size; ++i) {
						if (show_mode[i]) {
							int_mode = i;
						}
					}
					menu = create_menu(checkbox, &dammy,
						&show_mode[0], &show_mode[1], &show_mode[2],
						&start_game_flag, &analyze_flag,
						&use_ai_flag, &human_first, &human_second, &both_ai,
						&use_hint_flag, &normal_hint, &human_hint, &umigame_hint,
						&use_value_flag,
						&ai_level, &hint_level, &ai_book_accept,
						&start_book_learn_flag,
						&output_record_flag, &output_game_flag, &input_record_flag, &input_board_flag,
						&show_end_popup,
						&n_threads[0], &n_threads[1], &n_threads[2], &n_threads[3], &n_threads[4], &n_threads[5], &n_threads[6], &n_threads[7],
						language_acts, language_names);
				}
				initialize_draw(&initialize_future, &initializing, &initialize_failed, font50, font20, icon, logo, texture_loaded);
				if (!initializing) {
					bd.reset();
					bd.v = -inf;
					history.emplace_back(history_elem(bd, U""));
					history_place = 0;
					fork_mode = false;
					for (int i = 0; i < hw2; ++i) {
						hint_state[i] = hint_not_calculated_define;
					}
					for (int i = 0; i < hw2; ++i) {
						umigame_state[i] = hint_not_calculated_define;
					}
					human_value_state = hint_not_calculated_define;
					thread_pool.resize(n_threads_num[n_thread_idx]);
				}
			}
			else {
				lang_initialize_failed_draw(font50, font20, icon, logo);
			}
		}
		/*** initialize ***/

		/*** initialized ***/
		else {
			/**** when mode changed **/
			if (!show_mode[int_mode]) {
				for (int i = 0; i < mode_size; ++i) {
					if (show_mode[i]) {
						int_mode = i;
					}
				}
				menu = create_menu(checkbox, &dammy,
					&show_mode[0], &show_mode[1], &show_mode[2],
					&start_game_flag, &analyze_flag,
					&use_ai_flag, &human_first, &human_second, &both_ai,
					&use_hint_flag, &normal_hint, &human_hint, &umigame_hint,
					&use_value_flag,
					&ai_level, &hint_level, &ai_book_accept,
					&start_book_learn_flag,
					&output_record_flag, &output_game_flag, &input_record_flag, &input_board_flag,
					&show_end_popup,
					&n_threads[0], &n_threads[1], &n_threads[2], &n_threads[3], &n_threads[4], &n_threads[5], &n_threads[6], &n_threads[7],
					language_acts, language_names);
			}
			/**** when mode changed **/

			/*** thread ***/
			if (!n_threads[n_thread_idx]) {
				for (int i = 0; i < 8; ++i) {
					if (n_threads[i]) {
						n_thread_idx = i;
					}
				}
				thread_pool.resize(n_threads_num[n_thread_idx]);
			}
			/*** thread ***/

			/*** language ***/
			for (int i = 0; i < (int)language_names.size(); ++i) {
				if (language_acts[i] && language_names[i] != lang_name) {
					lang_name = language_names[i];
				}
			}
			/*** language ***/

			/*** analyzing ***/
			if (analyzing) {
				if (fork_mode) {
					if (analyze_idx == (int)fork_history.size()) {
						analyzing = false;
					}
					else {
						bd = fork_history[analyze_idx].b;
						history_place = fork_history[analyze_idx].b.n - 4;
						if (!analyze_state) {
							create_vacant_lst(bd);
							analyze_future = async(launch::async, analyze_search, fork_history[analyze_idx].b, ai_level);
							analyze_state = true;
						}
						else if (analyze_future.wait_for(chrono::seconds(0)) == future_status::ready) {
							fork_history[analyze_idx].b.v = analyze_future.get().value;
							analyze_state = false;
							++analyze_idx;
						}
					}
				}
				else {
					if (analyze_idx == (int)history.size()) {
						analyzing = false;
					}
					else {
						bd = history[analyze_idx].b;
						history_place = history[analyze_idx].b.n - 4;
						if (!analyze_state) {
							create_vacant_lst(bd);
							analyze_future = async(launch::async, analyze_search, history[analyze_idx].b, ai_level);
							analyze_state = true;
						}
						else if (analyze_future.wait_for(chrono::seconds(0)) == future_status::ready) {
							history[analyze_idx].b.v = analyze_future.get().value;
							analyze_state = false;
							++analyze_idx;
						}
					}
				}
			}
			/*** analyzing ***/

			if (!before_start_game) {
				unsigned long long legal = bd.mobility_ull();
				for (int cell = 0; cell < hw2; ++cell) {
					board_clicked[cell] = false;
				}
				if (!menu.active() && !analyzing && ((!use_ai_flag || (human_first && bd.p == black) || (human_second && bd.p == white)) || history_place != history[history.size() - 1].b.n - 4)) {
					for (int cell = 0; cell < hw2; ++cell) {
						board_clicked[cell] = board_cells[cell].leftClicked() && (1 & (legal >> cell));
						if (board_clicked[cell]) {
							global_searching = false;
						}
					}
				}
				if (not_finished(bd) && (!use_ai_flag || (human_first && bd.p == black) || (human_second && bd.p == white) || history_place != history[history.size() - 1].b.n - 4)) {
					/*** human moves ***/
					pair<bool, board> moved_board = move_board(bd, board_clicked);
					if (moved_board.first) {
						bool next_fork_mode = (!fork_mode && history_place != history[history.size() - 1].b.n - 4);
						bd = moved_board.second;
						bd.check_player();
						bd.v = -inf;
						if (fork_mode || next_fork_mode) {
							while (fork_history.size()) {
								if (fork_history[fork_history.size() - 1].b.n >= bd.n) {
									fork_history.pop_back();
								}
								else {
									break;
								}
							}
							if (!next_fork_mode) {
								fork_history.emplace_back(history_elem(bd, fork_history[fork_history.size() - 1].record + str_record(bd.policy)));
							}
							else {
								fork_history.emplace_back(history_elem(bd, history[find_history_idx(history, history_place)].record + str_record(bd.policy)));
								fork_mode = true;
							}
						}
						else {
							history.emplace_back(history_elem(bd, history[history.size() - 1].record + str_record(bd.policy)));
						}
						create_vacant_lst(bd);
						history_place = bd.n - 4;
						reset_hint(hint_state, hint_future);
						reset_umigame(umigame_state, umigame_future);
						reset_human_value(&human_value_state, &human_value_future);
					}
					/*** human moves ***/

					/*** hints ***/
					if (not_finished(bd) && use_hint_flag && !analyzing) {
						if (normal_hint){
							for (int cell = 0; cell < hw2; ++cell) {
								if ((1 & (legal >> cell)) && hint_state[cell] < hint_level * 2) {
									if (hint_state[cell] % 2 == 0) {
										if (hint_state[cell] == hint_level * 2 - 2) {
											hint_future[cell] = async(launch::async, hint_search, bd, hint_level, cell);
										}
										else {
											hint_future[cell] = async(launch::async, hint_search, bd, hint_state[cell] / 2, cell);
										}
										++hint_state[cell];
									}
									else if (hint_future[cell].wait_for(chrono::seconds(0)) == future_status::ready) {
										cell_value hint_result = hint_future[cell].get();
										if (global_searching) {
											if (hint_state[cell] == 1 || hint_result.depth == search_final_define) {
												hint_value[cell] = hint_result.value;
											}
											else {
												hint_value[cell] += hint_result.value;
												hint_value[cell] /= 2;
											}
											hint_depth[cell] = hint_result.depth;
										}
										if (hint_result.depth == search_final_define || hint_result.depth == search_book_define) {
											hint_state[cell] = hint_level * 2;
										}
										else {
											++hint_state[cell];
										}
									}
								}
							}
						}
						if (umigame_hint) {
							for (int cell = 0; cell < hw2; ++cell) {
								if ((1 & (legal >> cell))) {
									if (umigame_state[cell] == 0) {
										mobility m;
										calc_flip(&m, &bd, cell);
										board moved_b = bd.move_copy(&m);
										if (book.get(&moved_b) != -inf) {
											umigame_future[cell] = get_umigame(moved_b);
											umigame_state[cell] = 1;
										}
										else {
											umigame_state[cell] = 3;
										}
									}
									else if (umigame_state[cell] == 1) {
										if (umigame_future[cell].wait_for(chrono::seconds(0)) == future_status::ready) {
											umigame_value[cell] = umigame_future[cell].get();
											umigame_state[cell] = 2;
										}
									}
								}
							}
						}
						if (human_hint) {
							if (human_value_state == 0) {
								human_value_future = get_human_value(bd, human_value_depth, human_value_a, human_value);
								human_value_state = 1;
							}
							else if (human_value_state == 1) {
								if (human_value_future.wait_for(chrono::seconds(0)) == future_status::ready) {
									human_value_future.get();
									human_value_state = 2;
								}
							}
						}
					}
					/*** hints ***/
				}
				/*** ai plays ***/
				else if (not_finished(bd) && history[history.size() - 1].b.n - 4 == history_place) {
					if (ai_thinking) {
						if (ai_future.wait_for(chrono::seconds(0)) == future_status::ready) {
							search_result ai_result = ai_future.get();
							int sgn = (bd.p ? -1 : 1);
							mobility mob;
							calc_flip(&mob, &bd, ai_result.policy);
							bd.move(&mob);
							bd.check_player();
							bd.v = sgn * ai_result.value;
							history.emplace_back(history_elem(bd, history[history.size() - 1].record + str_record(bd.policy)));
							history_place = bd.n - 4;
							ai_value = ai_result.value;
							ai_thinking = false;
							reset_hint(hint_state, hint_future);
							reset_umigame(umigame_state, umigame_future);
							reset_human_value(&human_value_state, &human_value_future);
						}
					}
					else {
						global_searching = true;
						create_vacant_lst(bd);
						ai_future = async(launch::async, ai, bd, ai_level, ai_book_accept);
						ai_thinking = true;
					}
				}
				/*** ai plays ***/
			}

			/*** graph interaction ***/
			int former_history_place = history_place;
			if (showing_popup == 0 && !ai_thinking && starting_game == 0) {
				history_place = graph.update_place(history, fork_history, history_place);
				if (history_place != former_history_place) {
					if (ai_thinking) {
						reset_ai(&ai_thinking, &ai_future);
					}
					if (fork_history.size()) {
						if (history_place > fork_history[fork_history.size() - 1].b.n - 4) {
							history_place = fork_history[fork_history.size() - 1].b.n - 4;
						}
						if (history_place < fork_history[0].b.n - 4) {
							fork_mode = false;
						}
					}
					if (!fork_mode) {
						bd = history[find_history_idx(history, history_place)].b;
						create_vacant_lst(bd);
						fork_history.clear();
						reset_hint(hint_state, hint_future);
						reset_umigame(umigame_state, umigame_future);
						reset_human_value(&human_value_state, &human_value_future);
					}
					else {
						bd = fork_history[find_history_idx(fork_history, history_place)].b;
						create_vacant_lst(bd);
						reset_hint(hint_state, hint_future);
						reset_umigame(umigame_state, umigame_future);
						reset_human_value(&human_value_state, &human_value_future);
					}
				}
			}
			/*** graph interaction ***/

			showing_popup = max(0, showing_popup - 1);
			starting_game = max(0, starting_game - 1);

			/*** board draw ***/
			board_draw(board_cells, bd, int_mode, use_hint_flag, normal_hint, human_hint, umigame_hint,
				hint_state, hint_value, hint_depth, normal_hint_font, small_hint_font, font30, mini_hint_font, board_coord_font,
				before_start_game,
				umigame_state, umigame_value,
				human_value_state, human_value);
			/*** board draw ***/

			/*** before and after game ***/
			if (!before_start_game) {
				if (use_value_flag) {
					graph.draw(history, fork_history, history_place);
				}
				if (bd.p == vacant && !fork_mode && show_popup_flag && show_end_popup) {
					show_popup_flag = show_popup(bd, use_ai_flag, human_first, human_second, both_ai, font50, font30);
					showing_popup = 10;
				}
			}
			else {
				start_game_button.draw();
				how_to_use_button.draw();
				if (start_game_button.clicked()) {
					cerr << "start game!" << endl;
					before_start_game = false;
					starting_game = 10;
					if (history.size()) {
						history_place = history[history.size() - 1].b.n - 4;
					}
					else {
						history_place = 0;
					}
				}
				if (how_to_use_button.clicked()) {
					System::LaunchBrowser(U"https://www.egaroucid-app.nyanyan.dev/usage/");
				}
			}
			/*** before and after game ***/

			/*** menu buttons ***/
			if (start_game_flag) {
				cerr << "reset" << endl;
				bd.reset();
				bd.v = -inf;
				history.clear();
				history.emplace_back(history_elem(bd, U""));
				fork_history.clear();
				history_place = 0;
				fork_mode = false;
				before_start_game = true;
				showing_popup = false;
				show_popup_flag = true;
				reset_hint(hint_state, hint_future);
				reset_umigame(umigame_state, umigame_future);
				reset_human_value(&human_value_state, &human_value_future);
			}
			else if (analyze_flag) {
				analyzing = true;
				analyze_state = false;
				analyze_idx = 0;
				reset_hint(hint_state, hint_future);
				reset_umigame(umigame_state, umigame_future);
				reset_human_value(&human_value_state, &human_value_future);
			}
			else if (output_record_flag) {
				if (fork_mode) {
					Clipboard::SetText(fork_history[fork_history.size() - 1].record);
				}
				else {
					Clipboard::SetText(history[history.size() - 1].record);
				}
				cerr << "record copied" << endl;
			}
			else if (output_game_flag) {

			}
			else if (input_record_flag) {
				String record;
				if (Clipboard::GetText(record)) {
					vector<history_elem> n_history;
					if (import_record(record, &n_history)) {
						history.clear();
						fork_history.clear();
						for (history_elem elem : n_history) {
							history.emplace_back(elem);
						}
						bd = history[history.size() - 1].b;
						history_place = bd.n - 4;
						fork_mode = false;
						before_start_game = true;
						showing_popup = false;
						show_popup_flag = true;
						reset_hint(hint_state, hint_future);
						reset_umigame(umigame_state, umigame_future);
						reset_human_value(&human_value_state, &human_value_future);
					}
				}
			}
			else if (input_board_flag) {
				String board_str;
				if (Clipboard::GetText(board_str)) {
					pair<bool, board> imported = import_board(board_str);
					if (imported.first) {
						bd = imported.second;
						bd.v = -inf;
						history.clear();
						fork_history.clear();
						history.emplace_back(bd);
						history_place = bd.n - 4;
						fork_mode = false;
						before_start_game = true;
						showing_popup = false;
						show_popup_flag = true;
						reset_hint(hint_state, hint_future);
						reset_umigame(umigame_state, umigame_future);
						reset_human_value(&human_value_state, &human_value_future);
					}
				}
			}
			/*** menu buttons ***/

			menu.draw();
		}
		/*** initialized ***/
	}

}
