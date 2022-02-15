﻿#pragma once
#include <Siv3D.hpp> // OpenSiv3D v0.6.3
#include <vector>
#include "../board.hpp"
#include "gui_common.hpp"

using namespace std;

constexpr Color graph_color = Color(51, 51, 51);
constexpr Color graph_history_color = Palette::White;
constexpr Color graph_fork_color = Palette::Black;
constexpr Color graph_place_color = Palette::White;
constexpr double graph_transparency = 0.5;

class Graph {
public:
	int sx;
	int sy;
	int size_x;
	int size_y;
	int resolution;
	int font_size;
	Font font;

private:
	int y_max;
	int y_min;
	int dy;
	int dx;
	int adj_y;
	int adj_x;

public:
	void draw(vector<History_elem> nodes1, vector<History_elem> nodes2, int place) {
		calc_range(nodes1, nodes2);
		for (int y = 0; y <= y_max - y_min; y += resolution) {
			int yy = sy + y * dy + adj_y * y / (y_max - y_min);
			font(y_max - y).draw(sx - font(y_max - y).region(Point{ 0, 0 }).w - 12, yy - font(y_max - y).region(Point{ 0, 0 }).h / 2, graph_color);
			if (y_max - y == 0)
				Line{ sx, yy, sx + size_x, yy }.draw(2, graph_color);
			else
				Line{ sx, yy, sx + size_x, yy }.draw(1, graph_color);
		}
		for (int x = 0; x <= 60; x += 10) {
			font(x).draw(sx + x * dx + adj_x * x / 60 - font(x).region(Point{0, 0}).w, sy - 2 * font_size, graph_color);
			Line{ sx + x * dx + adj_x * x / 60, sy, sx + x * dx + adj_x * x / 60, sy + size_y }.draw(1, graph_color);
		}
		draw_graph(nodes1, graph_history_color, true);
		draw_graph(nodes2, graph_fork_color, true);
		int place_x = sx + place * dx + place * adj_x / 60;
		Circle(sx, sy, 10).draw(Palette::Black);
		Circle(sx, sy + size_y, 10).draw(Palette::White);
		Line(place_x, sy, place_x, sy + size_y).draw(3, graph_place_color);
		RoundRect(place_x - 9, sy + size_y, 18, 10, 3).draw(graph_place_color);
		Line(place_x - 6, sy + size_y + 3, place_x - 6, sy + size_y + 7).draw(2, graph_color);
		Line(place_x + 6, sy + size_y + 3, place_x + 6, sy + size_y + 7).draw(2, graph_color);
	}

	int update_place(vector<History_elem> nodes1, vector<History_elem> nodes2, int place) {
		if (Rect(sx - 30, sy, size_x + 40, size_y + 10).leftPressed()) {
			int cursor_x = Cursor::Pos().x;
			int min_err = INF;
			for (int i = 0; i < (int)nodes1.size(); ++i) {
				int x = sx + (nodes1[i].b.n - 4) * dx + (nodes1[i].b.n - 4) * adj_x / 60;
				if (abs(x - cursor_x) < min_err) {
					min_err = abs(x - cursor_x);
					place = nodes1[i].b.n - 4;
				}
			}
			for (int i = 0; i < (int)nodes2.size(); ++i) {
				int x = sx + (nodes2[i].b.n - 4) * dx + (nodes2[i].b.n - 4) * adj_x / 60;
				if (abs(x - cursor_x) < min_err) {
					min_err = abs(x - cursor_x);
					place = nodes2[i].b.n - 4;
				}
			}
		}
		return place;
	}

	bool clicked() {
		return Rect(sx - 30, sy, size_x + 40, size_y).leftClicked();
	}

	bool pressed() {
		return Rect(sx - 30, sy, size_x + 40, size_y).leftPressed();
	}

private:
	void calc_range(vector<History_elem> nodes1, vector<History_elem> nodes2) {
		y_min = 1000;
		y_max = -1000;
		for (const History_elem& b : nodes1) {
			if (b.b.v != -INF) {
				y_min = min(y_min, b.b.v);
				y_max = max(y_max, b.b.v);
			}
		}
		for (const History_elem& b : nodes2) {
			if (b.b.v != -INF) {
				y_min = min(y_min, b.b.v);
				y_max = max(y_max, b.b.v);
			}
		}
		y_min = min(y_min, 0);
		y_max = max(y_max, 0);
		y_min = y_min - resolution + abs(y_min) % resolution;
		y_max = y_max + resolution - abs(y_max) % resolution;
		dy = size_y / (y_max - y_min);
		dx = size_x / 60;
		adj_y = size_y - dy * (y_max - y_min);
		adj_x = size_x - dx * 60;
	}

	void draw_graph(vector<History_elem> nodes, Color color, bool show_not_calculated) {
		for (const History_elem& b : nodes) {
			if (b.b.v != -INF) {
				int yy = sy + (y_max - b.b.v) * dy + adj_y * (y_max - b.b.v) / (y_max - y_min);
				Circle{ sx + (b.b.n - 4) * dx + (b.b.n - 4) * adj_x / 60, yy, 3 }.draw(color);
			}
			else if (show_not_calculated) {
				int yy = sy + y_max * dy + adj_y * y_max / (y_max - y_min);
				Circle{ sx + (b.b.n - 4) * dx + (b.b.n - 4) * adj_x / 60, yy, 3 }.draw(color);
			}
		}
		int idx1 = 0, idx2 = 0;
		while (idx2 < (int)nodes.size()) {
			while (idx1 < (int)nodes.size()) {
				if (nodes[idx1].b.v != -INF)
					break;
				++idx1;
			}
			if (idx1 >= (int)nodes.size())
				break;
			int xx1 = sx + (nodes[idx1].b.n - 4) * dx + (nodes[idx1].b.n - 4) * adj_x / 60;
			int yy1 = sy + (y_max - nodes[idx1].b.v) * dy + adj_y * (y_max - nodes[idx1].b.v) / (y_max - y_min);
			idx2 = idx1 + 1;
			while (idx2 < (int)nodes.size()) {
				if (nodes[idx2].b.v != -INF)
					break;
				++idx2;
			}
			if (idx2 >= (int)nodes.size())
				break;
			int xx2 = sx + (nodes[idx2].b.n - 4) * dx + (nodes[idx2].b.n - 4) * adj_x / 60;
			int yy2 = sy + (y_max - nodes[idx2].b.v) * dy + adj_y * (y_max - nodes[idx2].b.v) / (y_max - y_min);
			Line(xx1, yy1, xx2, yy2).draw(2, ColorF(color, graph_transparency));
			idx1 = idx2;
		}
	}
};

