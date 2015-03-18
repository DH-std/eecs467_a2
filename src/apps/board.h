#ifndef BOARD_H
#define BOARD_H

#include <iostream>
#include <assert.h> 
#include <vector>
#include <string>
#include <math/point.hpp>

using namespace std;

typedef eecs467::Point<int> IntPoint;

class Board {

public: 
    vector<vector<char>> board;

    int num_R;
    int num_G;

    Board();
    Board(vector<vector<char>>& board_in);
    Board(Board& board_in);
    Board(string& s);

    bool set_board(vector<vector<char>>& board_in);
    bool set_board(string& s);
    bool reset_board();

    bool update_cell(int c, int r, char t);

    char operator()(int c, int r);
    char operator()(int idx);
    char operator()(IntPoint& p);

    bool board_full();

    void print_board();
};

#endif // BOARD_H
