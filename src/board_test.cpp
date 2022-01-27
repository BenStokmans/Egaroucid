#include <iostream>
#include <algorithm>
#include "setting.hpp"
#include "common.hpp"
#include "mobility.hpp"
#include "board.hpp"


inline void input_board(board *b, int ai_player){
    int i, j;
    char elem;
    int arr[hw2];
    for (i = 0; i < hw2; ++i){
        cin >> elem;
        if (elem == '.'){
            arr[i] = vacant;
        } else
            arr[i] = (int)elem - (int)'0';
    }
    b->translate_from_arr(arr, ai_player);
}

int main(){
    board b;
    mobility m;
    unsigned long long mob;
    int ai_player;
    while (true){
        cin >> ai_player;
        input_board(&b, ai_player);
        b.print();
        mob = b.mobility_ull();
        for (int i = 0; i < hw2; ++i){
            if (1 & (mob >> i)){
                m.calc_flip(b.b, b.w, i);
                b.move(&m);
                cerr << i << endl;
                b.print();
                cerr << endl;
                b.undo(&m);
                //b.print();
                cerr << endl;
            }
        }
    }
    return 0;
}