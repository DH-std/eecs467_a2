
#include <string>
#include <iostream>
#include "a2_board_state.h"

int main() {

    string mask_board = "mask_board";
    string mask_pick = "mask_pick";
    string hsv_R = "hsv_R";
    string hsv_G = "hsv_G";
    string hsv_B = "hsv_B";
    char turn_in = 'R';

    a2_board_state bs(mask_board, mask_pick, hsv_R, hsv_G, hsv_B, turn_in);

cout << ">>>>>>>>>>>>>>>>board inited " << endl;

    bs.detect();

    bs.board.print_board();
    cout << endl;

    

    cout << "cell center: ";
    for (int i = 0; i < 9; ++i) {
        cout << "(" << bs.cell_center[i].x << "," << bs.cell_center[i].y << ")";
    }
    cout << endl;

    cout << "to_pick: ";
    for (int i = 0; i < bs.to_pick.size(); ++i) {
        cout << "(" << bs.to_pick[i].x << "," << bs.to_pick[i].y << ")";
    }
    cout << endl;

    // bs.save_board();

}