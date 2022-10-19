﻿/*
    Egaroucid for Web Project

    @date 2021-2022
    @author Takuto Yamana (a.k.a Nyanyan)
    @license GPL-3.0 license
*/

#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "./../board.hpp"

using namespace std;



class Opening {
private:
	vector<pair<Board, string>> arr;
public:
	bool init(string file) {
		arr.clear();
		ifstream ifs(file);
		if (ifs.fail()) {
			cerr << "opening file " << file << " not found" << endl;
			return false;
		}
		string line;
		string name;
		int board_arr[HW2];
		int i;
		Board b;
		while (getline(ifs, line)) {
			for (i = 0; i < HW2; ++i) {
				if (line[i] == '0')
					board_arr[i] = BLACK;
				else if (line[i] == '1')
					board_arr[i] = WHITE;
				else
					board_arr[i] = VACANT;
			}
			b.translate_from_arr(board_arr, BLACK);
			name = line.substr(65, line.size());
			arr.push_back(make_pair(b, name));
			b.board_black_line_mirror();
			arr.push_back(make_pair(b, name));
			b.board_rotate_180();
			arr.push_back(make_pair(b, name));
			b.board_black_line_mirror();
			arr.push_back(make_pair(b, name));
		}
		return true;
	}

	inline string get(Board b, int p) {
		int i, j;
		bool flag;
		for (i = 0; i < (int)arr.size(); ++i) {
			if (p == BLACK && arr[i].first.player == b.opponent && arr[i].first.opponent == b.player)
				return arr[i].second;
			if (p == WHITE && arr[i].first.player == b.player && arr[i].first.opponent == b.opponent)
				return arr[i].second;
		}
		return "";
	}

};

Opening opening;
Opening opening_many;

bool opening_init(string lang) {
	return opening.init("resources/openings/" + lang + "/openings.txt") && opening_many.init("resources/openings/" + lang + "/openings_fork.txt");

}
