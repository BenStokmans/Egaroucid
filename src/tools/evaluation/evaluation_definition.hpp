#pragma once
#include "./../../engine/board.hpp"

/*
    @brief evaluation pattern definition
*/
#define ADJ_N_PATTERNS 12
#define ADJ_N_ADDITIONAL_EVALS 3
#define ADJ_N_MOBILITY_PATTERNS 1
#define ADJ_N_SYMMETRY_PATTERNS 46
#define ADJ_MAX_PATTERN_CELLS 10
#define ADJ_MAX_CELL_PATTERNS 8
#define ADJ_MAX_SURROUND 64
#define ADJ_MAX_CANPUT 35
#define ADJ_MAX_STONE_NUM 65
#define ADJ_MAX_MOBILITY_PATTERN 64
#define ADJ_MOBILITY_PATTERN_CELLS 6
#define ADJ_MAX_EVALUATE_IDX 59049

#define ADJ_N_EVAL (12 + 3 + 1)
#define ADJ_N_FEATURES (46 + 3 + 4)

#define ADJ_N_PHASES 60
#define ADJ_N_PHASE_DISCS (60 / ADJ_N_PHASES)

//#define ADJ_SCORE_MAX HW2

/*
    @brief value definition

    Raw score is STEP times larger than the real score.
*/
#define ADJ_STEP 256
#define ADJ_STEP_2 128
#define ADJ_STEP_SHIFT 8

#ifndef PNO
    /*
        @brief 3 ^ N definition
    */
    #define PNO 0
    #define P30 1
    #define P31 3
    #define P32 9
    #define P33 27
    #define P34 81
    #define P35 243
    #define P36 729
    #define P37 2187
    #define P38 6561
    #define P39 19683
    #define P310 59049

    /*
        @brief 4 ^ N definition
    */
    #define P40 1
    #define P41 4
    #define P42 16
    #define P43 64
    #define P44 256
    #define P45 1024
    #define P46 4096
    #define P47 16384
    #define P48 65536
#endif

#ifndef COORD_NO
    /*
        @brief coordinate definition
    */
    #define COORD_A1 63
    #define COORD_B1 62
    #define COORD_C1 61
    #define COORD_D1 60
    #define COORD_E1 59
    #define COORD_F1 58
    #define COORD_G1 57
    #define COORD_H1 56

    #define COORD_A2 55
    #define COORD_B2 54
    #define COORD_C2 53
    #define COORD_D2 52
    #define COORD_E2 51
    #define COORD_F2 50
    #define COORD_G2 49
    #define COORD_H2 48

    #define COORD_A3 47
    #define COORD_B3 46
    #define COORD_C3 45
    #define COORD_D3 44
    #define COORD_E3 43
    #define COORD_F3 42
    #define COORD_G3 41
    #define COORD_H3 40

    #define COORD_A4 39
    #define COORD_B4 38
    #define COORD_C4 37
    #define COORD_D4 36
    #define COORD_E4 35
    #define COORD_F4 34
    #define COORD_G4 33
    #define COORD_H4 32

    #define COORD_A5 31
    #define COORD_B5 30
    #define COORD_C5 29
    #define COORD_D5 28
    #define COORD_E5 27
    #define COORD_F5 26
    #define COORD_G5 25
    #define COORD_H5 24

    #define COORD_A6 23
    #define COORD_B6 22
    #define COORD_C6 21
    #define COORD_D6 20
    #define COORD_E6 19
    #define COORD_F6 18
    #define COORD_G6 17
    #define COORD_H6 16

    #define COORD_A7 15
    #define COORD_B7 14
    #define COORD_C7 13
    #define COORD_D7 12
    #define COORD_E7 11
    #define COORD_F7 10
    #define COORD_G7 9
    #define COORD_H7 8

    #define COORD_A8 7
    #define COORD_B8 6
    #define COORD_C8 5
    #define COORD_D8 4
    #define COORD_E8 3
    #define COORD_F8 2
    #define COORD_G8 1
    #define COORD_H8 0

    #define COORD_NO 64
#endif

constexpr int adj_pow3[11] = {P30, P31, P32, P33, P34, P35, P36, P37, P38, P39, P310};

/*
    @brief definition of patterns in evaluation function

    pattern -> coordinate

    @param n_cells              number of cells included in the pattern
    @param cells                coordinates of each cell
*/
struct Adj_Feature_to_coord{
    uint_fast8_t n_cells;
    uint_fast8_t cells[ADJ_MAX_PATTERN_CELLS];
};

constexpr Adj_Feature_to_coord adj_feature_to_coord[ADJ_N_SYMMETRY_PATTERNS] = {
    // 0 hv2
    {8, {COORD_A2, COORD_B2, COORD_C2, COORD_D2, COORD_E2, COORD_F2, COORD_G2, COORD_H2, COORD_NO, COORD_NO}}, // 0
    {8, {COORD_B1, COORD_B2, COORD_B3, COORD_B4, COORD_B5, COORD_B6, COORD_B7, COORD_B8, COORD_NO, COORD_NO}}, // 1
    {8, {COORD_A7, COORD_B7, COORD_C7, COORD_D7, COORD_E7, COORD_F7, COORD_G7, COORD_H7, COORD_NO, COORD_NO}}, // 2
    {8, {COORD_G1, COORD_G2, COORD_G3, COORD_G4, COORD_G5, COORD_G6, COORD_G7, COORD_G8, COORD_NO, COORD_NO}}, // 3

    // 1 hv3
    {8, {COORD_A3, COORD_B3, COORD_C3, COORD_D3, COORD_E3, COORD_F3, COORD_G3, COORD_H3, COORD_NO, COORD_NO}}, // 4
    {8, {COORD_C1, COORD_C2, COORD_C3, COORD_C4, COORD_C5, COORD_C6, COORD_C7, COORD_C8, COORD_NO, COORD_NO}}, // 5
    {8, {COORD_A6, COORD_B6, COORD_C6, COORD_D6, COORD_E6, COORD_F6, COORD_G6, COORD_H6, COORD_NO, COORD_NO}}, // 6
    {8, {COORD_F1, COORD_F2, COORD_F3, COORD_F4, COORD_F5, COORD_F6, COORD_F7, COORD_F8, COORD_NO, COORD_NO}}, // 7

    // 2 hv4
    {8, {COORD_A4, COORD_B4, COORD_C4, COORD_D4, COORD_E4, COORD_F4, COORD_G4, COORD_H4, COORD_NO, COORD_NO}}, // 8
    {8, {COORD_D1, COORD_D2, COORD_D3, COORD_D4, COORD_D5, COORD_D6, COORD_D7, COORD_D8, COORD_NO, COORD_NO}}, // 9
    {8, {COORD_A5, COORD_B5, COORD_C5, COORD_D5, COORD_E5, COORD_F5, COORD_G5, COORD_H5, COORD_NO, COORD_NO}}, // 10
    {8, {COORD_E1, COORD_E2, COORD_E3, COORD_E4, COORD_E5, COORD_E6, COORD_E7, COORD_E8, COORD_NO, COORD_NO}}, // 11

    // 3 d5
    {5, {COORD_D1, COORD_E2, COORD_F3, COORD_G4, COORD_H5, COORD_NO, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 12
    {5, {COORD_E1, COORD_D2, COORD_C3, COORD_B4, COORD_A5, COORD_NO, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 13
    {5, {COORD_A4, COORD_B5, COORD_C6, COORD_D7, COORD_E8, COORD_NO, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 14
    {5, {COORD_H4, COORD_G5, COORD_F6, COORD_E7, COORD_D8, COORD_NO, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 15

    // 4 d6
    {6, {COORD_C1, COORD_D2, COORD_E3, COORD_F4, COORD_G5, COORD_H6, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 16
    {6, {COORD_F1, COORD_E2, COORD_D3, COORD_C4, COORD_B5, COORD_A6, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 17
    {6, {COORD_A3, COORD_B4, COORD_C5, COORD_D6, COORD_E7, COORD_F8, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 18
    {6, {COORD_H3, COORD_G4, COORD_F5, COORD_E6, COORD_D7, COORD_C8, COORD_NO, COORD_NO, COORD_NO, COORD_NO}}, // 19

    // 5 d7
    {7, {COORD_B1, COORD_C2, COORD_D3, COORD_E4, COORD_F5, COORD_G6, COORD_H7, COORD_NO, COORD_NO, COORD_NO}}, // 20
    {7, {COORD_G1, COORD_F2, COORD_E3, COORD_D4, COORD_C5, COORD_B6, COORD_A7, COORD_NO, COORD_NO, COORD_NO}}, // 21
    {7, {COORD_A2, COORD_B3, COORD_C4, COORD_D5, COORD_E6, COORD_F7, COORD_G8, COORD_NO, COORD_NO, COORD_NO}}, // 22
    {7, {COORD_H2, COORD_G3, COORD_F4, COORD_E5, COORD_D6, COORD_C7, COORD_B8, COORD_NO, COORD_NO, COORD_NO}}, // 23

    // 6 d8
    {8, {COORD_A1, COORD_B2, COORD_C3, COORD_D4, COORD_E5, COORD_F6, COORD_G7, COORD_H8, COORD_NO, COORD_NO}}, // 24
    {8, {COORD_H1, COORD_G2, COORD_F3, COORD_E4, COORD_D5, COORD_C6, COORD_B7, COORD_A8, COORD_NO, COORD_NO}}, // 25

    // 7 edge + 2x
    {10, {COORD_B2, COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_G1, COORD_H1, COORD_G2}}, // 26
    {10, {COORD_B2, COORD_A1, COORD_A2, COORD_A3, COORD_A4, COORD_A5, COORD_A6, COORD_A7, COORD_A8, COORD_B7}}, // 27
    {10, {COORD_B7, COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_E8, COORD_F8, COORD_G8, COORD_H8, COORD_G7}}, // 28
    {10, {COORD_G2, COORD_H1, COORD_H2, COORD_H3, COORD_H4, COORD_H5, COORD_H6, COORD_H7, COORD_H8, COORD_G7}}, // 29

    // 8 triangle
    {10, {COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_A2, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_A4}}, // 30
    {10, {COORD_H1, COORD_G1, COORD_F1, COORD_E1, COORD_H2, COORD_G2, COORD_F2, COORD_H3, COORD_G3, COORD_H4}}, // 31
    {10, {COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_A7, COORD_B7, COORD_C7, COORD_A6, COORD_B6, COORD_A5}}, // 32
    {10, {COORD_H8, COORD_G8, COORD_F8, COORD_E8, COORD_H7, COORD_G7, COORD_F7, COORD_H6, COORD_G6, COORD_H5}}, // 33

    // 9 corner + block
    {10, {COORD_A1, COORD_C1, COORD_D1, COORD_E1, COORD_F1, COORD_H1, COORD_C2, COORD_D2, COORD_E2, COORD_F2}}, // 34
    {10, {COORD_A1, COORD_A3, COORD_A4, COORD_A5, COORD_A6, COORD_A8, COORD_B3, COORD_B4, COORD_B5, COORD_B6}}, // 35
    {10, {COORD_A8, COORD_C8, COORD_D8, COORD_E8, COORD_F8, COORD_H8, COORD_C7, COORD_D7, COORD_E7, COORD_F7}}, // 36
    {10, {COORD_H1, COORD_H3, COORD_H4, COORD_H5, COORD_H6, COORD_H8, COORD_G3, COORD_G4, COORD_G5, COORD_G6}}, // 37

    // 10 corner9
    {9, {COORD_A1, COORD_B1, COORD_C1, COORD_A2, COORD_B2, COORD_C2, COORD_A3, COORD_B3, COORD_C3, COORD_NO}}, // 38
    {9, {COORD_H1, COORD_G1, COORD_F1, COORD_H2, COORD_G2, COORD_F2, COORD_H3, COORD_G3, COORD_F3, COORD_NO}}, // 39
    {9, {COORD_A8, COORD_B8, COORD_C8, COORD_A7, COORD_B7, COORD_C7, COORD_A6, COORD_B6, COORD_C6, COORD_NO}}, // 40
    {9, {COORD_H8, COORD_G8, COORD_F8, COORD_H7, COORD_G7, COORD_F7, COORD_H6, COORD_G6, COORD_F6, COORD_NO}}, // 41

    // 11 narrow triangle
    {10, {COORD_A1, COORD_B1, COORD_C1, COORD_D1, COORD_E1, COORD_A2, COORD_B2, COORD_A3, COORD_A4, COORD_A5}}, // 42
    {10, {COORD_H1, COORD_G1, COORD_F1, COORD_E1, COORD_D1, COORD_H2, COORD_G2, COORD_H3, COORD_H4, COORD_H5}}, // 43
    {10, {COORD_A8, COORD_B8, COORD_C8, COORD_D8, COORD_E8, COORD_A7, COORD_B7, COORD_A6, COORD_A5, COORD_A4}}, // 44
    {10, {COORD_H8, COORD_G8, COORD_F8, COORD_E8, COORD_D8, COORD_H7, COORD_G7, COORD_H6, COORD_H5, COORD_H4}}  // 45
};

constexpr int adj_pattern_n_cells[ADJ_N_PATTERNS] = {8, 8, 8, 5, 6, 7, 8, 10, 10, 10, 9, 10};

constexpr int adj_rev_patterns[ADJ_N_PATTERNS][ADJ_MAX_PATTERN_CELLS] = {
    {7, 6, 5, 4, 3, 2, 1, 0}, // 0 hv2
    {7, 6, 5, 4, 3, 2, 1, 0}, // 1 hv3
    {7, 6, 5, 4, 3, 2, 1, 0}, // 2 hv4
    {4, 3, 2, 1, 0}, // 3 d5
    {5, 4, 3, 2, 1, 0}, // 4 d6
    {6, 5, 4, 3, 2, 1, 0}, // 5 d7
    {7, 6, 5, 4, 3, 2, 1, 0}, // 6 d8
    {9, 8, 7, 6, 5, 4, 3, 2, 1, 0}, // 7 edge + 2x
    {0, 4, 7, 9, 1, 5, 8, 2, 6, 3}, // 8 triangle
    {5, 4, 3, 2, 1, 0, 9, 8, 7, 6}, // 9 corner + block
    {0, 3, 6, 1, 4, 7, 2, 5, 8}, // 10 corner9
    {0, 5, 7, 8, 9, 1, 6, 2, 3, 4}  // 11 narrow triangle
};

constexpr int adj_rev_mobility_patterns[ADJ_N_MOBILITY_PATTERNS][ADJ_MOBILITY_PATTERN_CELLS] = {
    {0, 3, 5, 1, 4, 2}  // mobility triangle
};

/*
    @brief definition of patterns in evaluation function

    coordinate -> pattern

    @param feature              the index of feature
    @param x                    the offset value of the cell in this feature
*/
struct Adj_Coord_feature{
    uint_fast8_t feature;
    uint_fast16_t x;
};

/*
    @brief definition of patterns in evaluation function

    coordinate -> all patterns

    @param n_features           number of features the cell is used by
    @param features             information for each feature
*/
struct Adj_Coord_to_feature{
    uint_fast8_t n_features;
    Adj_Coord_feature features[ADJ_MAX_CELL_PATTERNS];
};

constexpr Adj_Coord_to_feature adj_coord_to_feature[HW2] = {
    { 8, {{24, P30}, {28, P31}, {29, P31}, {33, P39}, {36, P34}, {37, P34}, {41, P38}, {45, P39}}}, // COORD_H8
    { 6, {{ 3, P30}, {22, P30}, {28, P32}, {33, P38}, {41, P37}, {45, P38}, { 0, PNO}, { 0, PNO}}}, // COORD_G8
    { 7, {{ 7, P30}, {18, P30}, {28, P33}, {33, P37}, {36, P35}, {41, P36}, {45, P37}, { 0, PNO}}}, // COORD_F8
    { 7, {{11, P30}, {14, P30}, {28, P34}, {33, P36}, {36, P36}, {44, P35}, {45, P36}, { 0, PNO}}}, // COORD_E8
    { 7, {{ 9, P30}, {15, P30}, {28, P35}, {32, P36}, {36, P37}, {44, P36}, {45, P35}, { 0, PNO}}}, // COORD_D8
    { 7, {{ 5, P30}, {19, P30}, {28, P36}, {32, P37}, {36, P38}, {40, P36}, {44, P37}, { 0, PNO}}}, // COORD_C8
    { 6, {{ 1, P30}, {23, P30}, {28, P37}, {32, P38}, {40, P37}, {44, P38}, { 0, PNO}, { 0, PNO}}}, // COORD_B8
    { 8, {{25, P30}, {27, P31}, {28, P38}, {32, P39}, {35, P34}, {36, P39}, {40, P38}, {44, P39}}}, // COORD_A8
    { 6, {{ 2, P30}, {20, P30}, {29, P32}, {33, P35}, {41, P35}, {45, P34}, { 0, PNO}, { 0, PNO}}}, // COORD_H7
    { 8, {{ 2, P31}, { 3, P31}, {24, P31}, {28, P30}, {29, P30}, {33, P34}, {41, P34}, {45, P33}}}, // COORD_G7
    { 6, {{ 2, P32}, { 7, P31}, {22, P31}, {33, P33}, {36, P30}, {41, P33}, { 0, PNO}, { 0, PNO}}}, // COORD_F7
    { 5, {{ 2, P33}, {11, P31}, {15, P31}, {18, P31}, {36, P31}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E7
    { 5, {{ 2, P34}, { 9, P31}, {14, P31}, {19, P31}, {36, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D7
    { 6, {{ 2, P35}, { 5, P31}, {23, P31}, {32, P33}, {36, P33}, {40, P33}, { 0, PNO}, { 0, PNO}}}, // COORD_C7
    { 8, {{ 1, P31}, { 2, P36}, {25, P31}, {27, P30}, {28, P39}, {32, P34}, {40, P34}, {44, P33}}}, // COORD_B7
    { 6, {{ 2, P37}, {21, P30}, {27, P32}, {32, P35}, {40, P35}, {44, P34}, { 0, PNO}, { 0, PNO}}}, // COORD_A7
    { 7, {{ 6, P30}, {16, P30}, {29, P33}, {33, P32}, {37, P35}, {41, P32}, {45, P32}, { 0, PNO}}}, // COORD_H6
    { 6, {{ 3, P32}, { 6, P31}, {20, P31}, {33, P31}, {37, P30}, {41, P31}, { 0, PNO}, { 0, PNO}}}, // COORD_G6
    { 5, {{ 6, P32}, { 7, P32}, {15, P32}, {24, P32}, {41, P30}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_F6
    { 4, {{ 6, P33}, {11, P32}, {19, P32}, {22, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E6
    { 4, {{ 6, P34}, { 9, P32}, {18, P32}, {23, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D6
    { 5, {{ 5, P32}, { 6, P35}, {14, P32}, {25, P32}, {40, P30}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_C6
    { 6, {{ 1, P32}, { 6, P36}, {21, P31}, {32, P31}, {35, P30}, {40, P31}, { 0, PNO}, { 0, PNO}}}, // COORD_B6
    { 7, {{ 6, P37}, {17, P30}, {27, P33}, {32, P32}, {35, P35}, {40, P32}, {44, P32}, { 0, PNO}}}, // COORD_A6
    { 7, {{10, P30}, {12, P30}, {29, P34}, {33, P30}, {37, P36}, {43, P30}, {45, P31}, { 0, PNO}}}, // COORD_H5
    { 5, {{ 3, P33}, {10, P31}, {15, P33}, {16, P31}, {37, P31}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_G5
    { 4, {{ 7, P33}, {10, P32}, {19, P33}, {20, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_F5
    { 4, {{10, P33}, {11, P33}, {23, P33}, {24, P33}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E5
    { 4, {{ 9, P33}, {10, P34}, {22, P33}, {25, P33}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D5
    { 4, {{ 5, P33}, {10, P35}, {18, P33}, {21, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_C5
    { 5, {{ 1, P33}, {10, P36}, {14, P33}, {17, P31}, {35, P31}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_B5
    { 7, {{10, P37}, {13, P30}, {27, P34}, {32, P30}, {35, P36}, {42, P30}, {44, P31}, { 0, PNO}}}, // COORD_A5
    { 7, {{ 8, P30}, {15, P34}, {29, P35}, {31, P30}, {37, P37}, {43, P31}, {45, P30}, { 0, PNO}}}, // COORD_H4
    { 5, {{ 3, P34}, { 8, P31}, {12, P31}, {19, P34}, {37, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_G4
    { 4, {{ 7, P34}, { 8, P32}, {16, P32}, {23, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_F4
    { 4, {{ 8, P33}, {11, P34}, {20, P33}, {25, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E4
    { 4, {{ 8, P34}, { 9, P34}, {21, P33}, {24, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D4
    { 4, {{ 5, P34}, { 8, P35}, {17, P32}, {22, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_C4
    { 5, {{ 1, P34}, { 8, P36}, {13, P31}, {18, P34}, {35, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_B4
    { 7, {{ 8, P37}, {14, P34}, {27, P35}, {30, P30}, {35, P37}, {42, P31}, {44, P30}, { 0, PNO}}}, // COORD_A4
    { 7, {{ 4, P30}, {19, P35}, {29, P36}, {31, P32}, {37, P38}, {39, P32}, {43, P32}, { 0, PNO}}}, // COORD_H3
    { 6, {{ 3, P35}, { 4, P31}, {23, P35}, {31, P31}, {37, P33}, {39, P31}, { 0, PNO}, { 0, PNO}}}, // COORD_G3
    { 5, {{ 4, P32}, { 7, P35}, {12, P32}, {25, P35}, {39, P30}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_F3
    { 4, {{ 4, P33}, {11, P35}, {16, P33}, {21, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E3
    { 4, {{ 4, P34}, { 9, P35}, {17, P33}, {20, P34}, { 0, PNO}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D3
    { 5, {{ 4, P35}, { 5, P35}, {13, P32}, {24, P35}, {38, P30}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_C3
    { 6, {{ 1, P35}, { 4, P36}, {22, P35}, {30, P31}, {35, P33}, {38, P31}, { 0, PNO}, { 0, PNO}}}, // COORD_B3
    { 7, {{ 4, P37}, {18, P35}, {27, P36}, {30, P32}, {35, P38}, {38, P32}, {42, P32}, { 0, PNO}}}, // COORD_A3
    { 6, {{ 0, P30}, {23, P36}, {29, P37}, {31, P35}, {39, P35}, {43, P34}, { 0, PNO}, { 0, PNO}}}, // COORD_H2
    { 8, {{ 0, P31}, { 3, P36}, {25, P36}, {26, P30}, {29, P39}, {31, P34}, {39, P34}, {43, P33}}}, // COORD_G2
    { 6, {{ 0, P32}, { 7, P36}, {21, P35}, {31, P33}, {34, P30}, {39, P33}, { 0, PNO}, { 0, PNO}}}, // COORD_F2
    { 5, {{ 0, P33}, {11, P36}, {12, P33}, {17, P34}, {34, P31}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_E2
    { 5, {{ 0, P34}, { 9, P36}, {13, P33}, {16, P34}, {34, P32}, { 0, PNO}, { 0, PNO}, { 0, PNO}}}, // COORD_D2
    { 6, {{ 0, P35}, { 5, P36}, {20, P35}, {30, P33}, {34, P33}, {38, P33}, { 0, PNO}, { 0, PNO}}}, // COORD_C2
    { 8, {{ 0, P36}, { 1, P36}, {24, P36}, {26, P39}, {27, P39}, {30, P34}, {38, P34}, {42, P33}}}, // COORD_B2
    { 6, {{ 0, P37}, {22, P36}, {27, P37}, {30, P35}, {38, P35}, {42, P34}, { 0, PNO}, { 0, PNO}}}, // COORD_A2
    { 8, {{25, P37}, {26, P31}, {29, P38}, {31, P39}, {34, P34}, {37, P39}, {39, P38}, {43, P39}}}, // COORD_H1
    { 6, {{ 3, P37}, {21, P36}, {26, P32}, {31, P38}, {39, P37}, {43, P38}, { 0, PNO}, { 0, PNO}}}, // COORD_G1
    { 7, {{ 7, P37}, {17, P35}, {26, P33}, {31, P37}, {34, P35}, {39, P36}, {43, P37}, { 0, PNO}}}, // COORD_F1
    { 7, {{11, P37}, {13, P34}, {26, P34}, {31, P36}, {34, P36}, {42, P35}, {43, P36}, { 0, PNO}}}, // COORD_E1
    { 7, {{ 9, P37}, {12, P34}, {26, P35}, {30, P36}, {34, P37}, {42, P36}, {43, P35}, { 0, PNO}}}, // COORD_D1
    { 7, {{ 5, P37}, {16, P35}, {26, P36}, {30, P37}, {34, P38}, {38, P36}, {42, P37}, { 0, PNO}}}, // COORD_C1
    { 6, {{ 1, P37}, {20, P36}, {26, P37}, {30, P38}, {38, P37}, {42, P38}, { 0, PNO}, { 0, PNO}}}, // COORD_B1
    { 8, {{24, P37}, {26, P38}, {27, P38}, {30, P39}, {34, P39}, {35, P39}, {38, P38}, {42, P39}}}  // COORD_A1
};

constexpr int adj_eval_sizes[ADJ_N_EVAL] = {
    P38, P38, P38, P35, P36, P37, P38, 
    P310, P310, P310, P39, P310, 
    ADJ_MAX_SURROUND * ADJ_MAX_SURROUND, 
    ADJ_MAX_CANPUT * ADJ_MAX_CANPUT, 
    ADJ_MAX_STONE_NUM * ADJ_MAX_STONE_NUM,
    ADJ_MAX_MOBILITY_PATTERN * ADJ_MAX_MOBILITY_PATTERN
};

constexpr int adj_feature_to_eval_idx[ADJ_N_FEATURES] = {
    0, 0, 0, 0, 
    1, 1, 1, 1, 
    2, 2, 2, 2, 
    3, 3, 3, 3, 
    4, 4, 4, 4, 
    5, 5, 5, 5, 
    6, 6, 
    7, 7, 7, 7, 
    8, 8, 8, 8, 
    9, 9, 9, 9, 
    10, 10, 10, 10, 
    11, 11, 11, 11, 
    12, 
    13, 
    14,
    15, 15, 15, 15
};

/*
    @brief calculate surround value used in evaluation function

    @param player               a bitboard representing player
    @param empties              a bitboard representing empties
    @return surround value
*/
inline int calc_surround(const uint64_t player, const uint64_t empties){
    const u64_4 shift(1, HW, HW_M1, HW_P1);
    const u64_4 mask(0x7E7E7E7E7E7E7E7EULL, 0x00FFFFFFFFFFFF00ULL, 0x007E7E7E7E7E7E00ULL, 0x007E7E7E7E7E7E00ULL);
    u64_4 pl(player);
    pl = pl & mask;
    return pop_count_ull(empties & all_or((pl << shift) | (pl >> shift)));
}

int adj_calc_surround_feature(Board *board){
    return calc_surround(board->player, ~(board->player | board->opponent)) * ADJ_MAX_SURROUND + calc_surround(board->opponent, ~(board->player | board->opponent));
}

int adj_calc_legal_feature(Board *board){
    return pop_count_ull(calc_legal(board->player, board->opponent)) * ADJ_MAX_CANPUT + pop_count_ull(calc_legal(board->opponent, board->player));
}

int adj_calc_num_feature(Board *board){
    return pop_count_ull(board->player) * ADJ_MAX_STONE_NUM + pop_count_ull(board->opponent);
}

void adj_calc_mobility_features(Board *board, uint16_t res[], int *idx){
    uint64_t p = calc_legal(board->player, board->opponent);
    uint64_t o = calc_legal(board->opponent, board->player);
    uint64_t pp = p & 0x0703010000010307ULL;
    uint64_t oo = o & 0x0703010000010307ULL;
    pp *= 0x0000000000200841ULL;
    oo *= 0x0000000000200841ULL;
    pp &= 0x3F0000000001F800ULL;
    oo &= 0x3F0000000001F800ULL;
    uint16_t p1, p2, o1, o2;
    p1 = pp >> 56;
    p2 = pp >> 11;
    o1 = oo >> 56;
    o2 = oo >> 11;
    res[(*idx)++] = p1 * ADJ_MAX_MOBILITY_PATTERN + o1;
    res[(*idx)++] = p2 * ADJ_MAX_MOBILITY_PATTERN + o2;

    p = horizontal_mirror(p);
    o = horizontal_mirror(o);
    pp = p & 0x0703010000010307ULL;
    oo = o & 0x0703010000010307ULL;
    pp *= 0x0000000000200841ULL;
    oo *= 0x0000000000200841ULL;
    pp &= 0x3F0000000001F800ULL;
    oo &= 0x3F0000000001F800ULL;
    p1 = pp >> 56;
    p2 = pp >> 11;
    o1 = oo >> 56;
    o2 = oo >> 11;
    res[(*idx)++] = p1 * ADJ_MAX_MOBILITY_PATTERN + o1;
    res[(*idx)++] = p2 * ADJ_MAX_MOBILITY_PATTERN + o2;
}

inline int adj_create_canput_line_h(uint64_t b, uint64_t w, int t){
    return (((w >> (HW * t)) & 0b11111111) << HW) | ((b >> (HW * t)) & 0b11111111);
}

inline int adj_create_canput_line_v(uint64_t b, uint64_t w, int t){
    return (join_v_line(w, t) << HW) | join_v_line(b, t);
}

inline int adj_pick_pattern(const uint_fast8_t b_arr[], int pattern_idx){
    int res = 0;
    for (int i = 0; i < adj_feature_to_coord[pattern_idx].n_cells; ++i){
        res *= 3;
        res += b_arr[adj_feature_to_coord[pattern_idx].cells[i]];
    }
    return res;
}

void adj_calc_features(Board *board, uint16_t res[]){
    uint_fast8_t b_arr[HW2];
    board->translate_to_arr_player(b_arr);
    int idx = 0;
    for (int i = 0; i < ADJ_N_SYMMETRY_PATTERNS; ++i)
        res[idx++] = adj_pick_pattern(b_arr, i);
    res[idx++] = adj_calc_surround_feature(board);
    res[idx++] = adj_calc_legal_feature(board);
    res[idx++] = adj_calc_num_feature(board);
    adj_calc_mobility_features(board, res, &idx);
}

int adj_pick_digit3(int num, int d, int n_digit){
    num /= adj_pow3[n_digit - 1 - d];
    return num % 3;
}

int adj_pick_digit2(int num, int d){
    return 1 & (num >> d);
}

uint16_t adj_calc_rev_idx(int feature, int idx){
    uint16_t res = 0;
    if (feature < ADJ_N_PATTERNS){
        for (int i = 0; i < adj_pattern_n_cells[feature]; ++i){
            res += adj_pick_digit3(idx, adj_rev_patterns[feature][i], adj_pattern_n_cells[feature]) * adj_pow3[adj_pattern_n_cells[feature] - 1 - i];
        }
    } else if (feature < ADJ_N_PATTERNS + ADJ_N_ADDITIONAL_EVALS){
        res = idx;
    } else{
        for (int i = 0; i < ADJ_MOBILITY_PATTERN_CELLS; ++i){
            res |= adj_pick_digit2(idx, i) << adj_rev_mobility_patterns[0][i];
            res |= adj_pick_digit2(idx, i + ADJ_MOBILITY_PATTERN_CELLS) << (adj_rev_mobility_patterns[0][i] + ADJ_MOBILITY_PATTERN_CELLS);
        }
    }
    return res;
}