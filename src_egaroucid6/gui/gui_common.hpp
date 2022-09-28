﻿#pragma once
#include <iostream>
#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include "./ai.hpp"
#include "version.hpp"

using namespace std;

// graph definition
#define GRAPH_IGNORE_VALUE INF

// scene definition
#define SCENE_FADE_TIME 200

// coordinate definition
#define WINDOW_SIZE_X 800
#define WINDOW_SIZE_Y 500
#define PADDING 20
constexpr int LEFT_LEFT = PADDING;
constexpr int LEFT_RIGHT = WINDOW_SIZE_X / 2 - PADDING;
constexpr int LEFT_CENTER = (LEFT_LEFT + LEFT_RIGHT) / 2;
constexpr int RIGHT_LEFT = WINDOW_SIZE_X / 2 + PADDING;
constexpr int RIGHT_RIGHT = WINDOW_SIZE_X - PADDING;
constexpr int RIGHT_CENTER = (RIGHT_LEFT + RIGHT_RIGHT) / 2;
constexpr int X_CENTER = WINDOW_SIZE_X / 2;
constexpr int Y_CENTER = WINDOW_SIZE_Y / 2;

// error definition
#define ERR_OK 0
#define ERR_LANG_LIST_NOT_LOADED 1
#define ERR_LANG_JSON_NOT_LOADED 2
#define ERR_LANG_NOT_LOADED 3
#define ERR_TEXTURE_NOT_LOADED 4
#define ERR_OPENING_NOT_LOADED 5
#define ERR_EVAL_FILE_NOT_IMPORTED 1
#define ERR_BOOK_FILE_NOT_IMPORTED 2
#define ERR_IMPORT_SETTINGS 1

// constant definition
#define SHOW_ALL_HINT 35
#define UPDATE_CHECK_ALREADY_UPDATED 0
#define UPDATE_CHECK_UPDATE_FOUND 1

// board drawing constants
#define BOARD_SIZE 400
#define BOARD_COORD_SIZE 20
#define DISC_SIZE 20
#define LEGAL_SIZE 7
#define BOARD_CELL_FRAME_WIDTH 2
#define BOARD_DOT_SIZE 5
#define BOARD_ROUND_FRAME_WIDTH 10
#define BOARD_ROUND_DIAMETER 20
#define BOARD_SY 60
constexpr int BOARD_SX = LEFT_LEFT + BOARD_COORD_SIZE;
constexpr int BOARD_CELL_SIZE = BOARD_SIZE / HW;

// graph drawing constants
#define GRAPH_RESOLUTION 8
constexpr int GRAPH_SX = BOARD_SX + BOARD_SIZE + 50;
constexpr int GRAPH_SY = Y_CENTER + 30;
constexpr int GRAPH_WIDTH = WINDOW_SIZE_X - GRAPH_SX - 20;
constexpr int GRAPH_HEIGHT = WINDOW_SIZE_Y - GRAPH_SY - 40;

// info drawing constants
#define INFO_SY 35
#define INFO_DISC_RADIUS 12
constexpr int INFO_SX = BOARD_SX + BOARD_SIZE + 25;

// graph mode constants
#define GRAPH_MODE_NORMAL 0
#define GRAPH_MODE_INSPECT 1

// button press constants
#define BUTTON_NOT_PUSHED 0
#define BUTTON_LONG_PRESS_THRESHOLD 500

// hint constants
#define HINT_NOT_CALCULATING -1
#define HINT_INIT_VALUE -INF
#define HINT_TYPE_BOOK 1000
#define HINT_INF_LEVEL 100
#define HINT_MAX_LEVEL 60

// analyze constants
#define ANALYZE_SIZE 62

// export game constants
#define EXPORT_GAME_PLAYER_WIDTH 300
#define EXPORT_GAME_PLAYER_HEIGHT 30
#define EXPORT_GAME_MEMO_WIDTH 600
#define EXPORT_GAME_MEMO_HEIGHT 250
#define EXPORT_GAME_RADIUS 15

// import game constants
#define IMPORT_GAME_N_GAMES_ON_WINDOW 7
#define IMPORT_GAME_SX 30
#define IMPORT_GAME_SY 65
#define IMPORT_GAME_HEIGHT 45
#define IMPORT_GAME_PLAYER_WIDTH 220
#define IMPORT_GAME_PLAYER_HEIGHT 25
#define IMPORT_GAME_SCORE_WIDTH 60
#define IMPORT_GAME_WINNER_BLACK 0
#define IMPORT_GAME_WINNER_WHITE 1
#define IMPORT_GAME_WINNER_DRAW 2
#define IMPORT_GAME_BUTTON_SX 660
#define IMPORT_GAME_BUTTON_WIDTH 100
#define IMPORT_GAME_BUTTON_HEIGHT 25
#define IMPORT_GAME_BUTTON_RADIUS 7
#define IMPORT_GAME_DATE_WIDTH 120
constexpr int IMPORT_GAME_BUTTON_SY = (IMPORT_GAME_HEIGHT - IMPORT_GAME_BUTTON_HEIGHT) / 2;
constexpr int IMPORT_GAME_WIDTH = WINDOW_SIZE_X - IMPORT_GAME_SX * 2;

// game saving constants
#define GAME_DATE U"date"
#define GAME_BLACK_PLAYER U"black_player"
#define GAME_WHITE_PLAYER U"white_player"
#define GAME_BLACK_DISCS U"black_discs"
#define GAME_WHITE_DISCS U"white_discs"
#define GAME_MEMO U"memo"
#define GAME_BOARD_PLAYER U"board_player"
#define GAME_BOARD_OPPONENT U"board_opponent"
#define GAME_PLAYER U"player"
#define GAME_VALUE U"value"
#define GAME_LEVEL U"level"
#define GAME_POLICY U"policy"
#define GAME_NEXT_POLICY U"next_policy"
#define GAME_DISCS_UNDEFINED -1
#define GAME_MEMO_SUMMARY_SIZE 40


// back button constants
#define BACK_BUTTON_WIDTH 200
#define BACK_BUTTON_HEIGHT 50
#define BACK_BUTTON_SY 420
#define BACK_BUTTON_RADIUS 20
constexpr int BACK_BUTTON_SX = X_CENTER - BACK_BUTTON_WIDTH / 2;

// go/back button constants
#define GO_BACK_BUTTON_WIDTH 200
#define GO_BACK_BUTTON_HEIGHT 50
#define GO_BACK_BUTTON_SY 420
#define GO_BACK_BUTTON_RADIUS 20
constexpr int GO_BACK_BUTTON_GO_SX = X_CENTER + 10;
constexpr int GO_BACK_BUTTON_BACK_SX = X_CENTER - GO_BACK_BUTTON_WIDTH - 10;

// 3 buttons constants
#define BUTTON3_WIDTH 200
#define BUTTON3_HEIGHT 50
#define BUTTON3_SY 420
#define BUTTON3_RADIUS 20
constexpr int BUTTON3_1_SX = X_CENTER - BUTTON3_WIDTH * 3 / 2 - 10;
constexpr int BUTTON3_2_SX = X_CENTER - BUTTON3_WIDTH / 2;
constexpr int BUTTON3_3_SX = X_CENTER + BUTTON3_WIDTH / 2 + 10;

// 2 button vertical constants
#define BUTTON2_VERTICAL_WIDTH 200
#define BUTTON2_VERTICAL_HEIGHT 50
#define BUTTON2_VERTICAL_1_SY 350
#define BUTTON2_VERTICAL_2_SY 420
#define BUTTON2_VERTICAL_SX 520
#define BUTTON2_VERTICAL_RADIUS 20

struct History_elem {
	Board board;
	int player;
	int v;
	int level;
	int policy;
	int next_policy;
	string opening_name;

	History_elem() {
		reset();
	}

	void reset() {
		board.reset();
		player = 0;
		v = GRAPH_IGNORE_VALUE;
		policy = -1;
		next_policy = -1;
		level = -1;
	}

	void set(Board b, int p, int vv, int l, int pl, int npl, string o) {
		board = b;
		player = p;
		v = vv;
		level = l;
		policy = pl;
		next_policy = npl;
		opening_name = o;
	}
};

struct Colors {
	Color green{ Color(36, 153, 114) };
	Color black{ Palette::Black };
	Color white{ Palette::White };
	Color dark_gray{ Color(51, 51, 51) };
	Color cyan{ Palette::Cyan };
	Color red{ Palette::Red };
	Color light_cyan{ Palette::Lightcyan };
	Color chocolate{ Color(210, 105, 30) };
	Color darkred{ Color(139, 0, 0) };
	Color darkblue{ Color(0, 0, 139) };
};

struct Directories {
	string document_dir;
	string appdata_dir;
	string eval_file;
};

struct Resources {
	vector<string> language_names;
	Texture icon;
	Texture logo;
	Texture checkbox;
};

struct Settings {
	int n_threads;
	bool auto_update_check;
	string lang_name;
	string book_file;
	bool use_book;
	int level;
	bool ai_put_black;
	bool ai_put_white;
	bool use_disc_hint;
	bool use_umigame_value;
	int n_disc_hint;
	bool show_legal;
	bool show_graph;
	bool show_opening_on_cell;
	bool show_log;
	int book_learn_depth;
	int book_learn_error;
};

struct Fonts {
	Font font50{ 50 };
	Font font40{ 40 };
	Font font30{ 30 };
	Font font25{ 25 };
	Font font20{ 20 };
	Font font15{ 15 };
	Font font13{ 13 };
	Font font12{ 12 };
	Font font10{ 10 };
	Font font15_bold{ 15, Typeface::Bold };
	Font font13_heavy{ 13, Typeface::Heavy };
	Font font9_bold{ 9, Typeface::Bold };
};

struct Menu_elements {
	bool dummy;

	// 対局
	bool start_game;
	bool analyze;

	// 設定
	// AIの設定
	bool use_book;
	int level;
	int n_threads;
	// 着手
	bool ai_put_black;
	bool ai_put_white;

	// 表示
	bool use_disc_hint;
	int n_disc_hint;
	bool use_umigame_value;
	bool show_legal;
	bool show_graph;
	bool show_opening_on_cell;
	bool show_log;

	// 定石
	bool book_start_learn;
	int book_learn_depth;
	int book_learn_error;
	bool book_import;
	bool book_reference;

	// 入出力
	// 入力
	bool input_transcript;
	bool input_board;
	bool edit_board;
	bool input_game;
	// 出力
	bool copy_transcript;
	bool save_game;

	// 操作
	bool stop_calculating;
	bool forward;
	bool backward;
	// 変換
	bool convert_180;
	bool convert_blackline;
	bool convert_whiteline;

	// ヘルプ
	bool usage;
	bool bug_report;
	bool auto_update_check;
	bool license_egaroucid;
	bool license_siv3d;

	// language
	bool languages[200];

	void init(Settings* settings, Resources* resources) {
		dummy = false;

		start_game = false;
		analyze = false;

		use_book = settings->use_book;
		level = settings->level;
		n_threads = settings->n_threads;
		ai_put_black = settings->ai_put_black;
		ai_put_white = settings->ai_put_white;

		use_disc_hint = settings->use_disc_hint;
		n_disc_hint = settings->n_disc_hint;
		use_umigame_value = settings->use_umigame_value;
		show_legal = settings->show_legal;
		show_graph = settings->show_graph;
		show_opening_on_cell = settings->show_opening_on_cell;
		show_log = settings->show_log;

		book_start_learn = false;
		book_learn_depth = settings->book_learn_depth;
		book_learn_error = settings->book_learn_error;
		book_import = false;
		book_reference = false;

		input_transcript = false;
		input_board = false;
		edit_board = false;
		input_game = false;
		copy_transcript = false;
		save_game = false;

		stop_calculating = false;
		forward = false;
		backward = false;
		convert_180 = false;
		convert_blackline = false;
		convert_whiteline = false;

		usage = false;
		bug_report = false;
		auto_update_check = settings->auto_update_check;
		license_egaroucid = false;
		license_siv3d = false;

		bool lang_found = false;
		for (int i = 0; i < resources->language_names.size(); ++i) {
			if (resources->language_names[i] == settings->lang_name) {
				lang_found = true;
				languages[i] = true;
			}
			else {
				languages[i] = false;
			}
		}
		if (!lang_found) {
			settings->lang_name = resources->language_names[0];
			languages[0] = true;
		}
	}
};

struct Graph_resources {
	vector<History_elem> nodes[2];
	int n_discs;
	int delta;
	int put_mode;
	bool need_init;

	Graph_resources() {
		init();
	}

	void init() {
		nodes[0].clear();
		nodes[1].clear();
		n_discs = 4;
		delta = 0;
		put_mode = 0;
		need_init = true;
	}

	int node_find(int mode, int n_discs) {
		for (int i = 0; i < (int)nodes[mode].size(); ++i) {
			if (nodes[mode][i].board.n_discs() == n_discs) {
				return i;
			}
		}
		return -1;
	}
};

struct Common_resources {
	Colors colors;
	Directories directories;
	Resources resources;
	Settings settings;
	Fonts fonts;
	Menu_elements menu_elements;
	Menu menu;
	History_elem history_elem;
	Graph_resources graph_resources;
};

struct Hint_info {
	double value;
	int cell;
	int type;
};

struct Move_board_button_status {
	uint64_t left_pushed{ BUTTON_NOT_PUSHED };
	uint64_t right_pushed{ BUTTON_NOT_PUSHED };
};

struct Analyze_info {
	int idx;
	int sgn;
	Board board;
};

struct AI_status {
	bool ai_thinking{ false };
	future<Search_result> ai_future;

	bool hint_calculating{ false };
	int hint_level{ HINT_NOT_CALCULATING };
	future<Search_result> hint_future[HW2];
	vector<pair<int, function<Search_result()>>> hint_task_stack;
	bool hint_use[HW2];
	double hint_values[HW2];
	int hint_types[HW2];
	bool hint_available{ false };
	bool hint_use_stable[HW2];
	double hint_values_stable[HW2];
	int hint_types_stable[HW2];
	bool hint_use_multi_thread;
	int hint_n_doing_tasks;

	bool analyzing{ false };
	future<Search_result> analyze_future[ANALYZE_SIZE];
	int analyze_sgn[ANALYZE_SIZE];
	vector<pair<Analyze_info, function<Search_result()>>> analyze_task_stack;
};

struct Game_abstract {
	String black_player;
	String white_player;
	int black_score;
	int white_score;
	String memo;
	String date;
};

using App = SceneManager<String, Common_resources>;
