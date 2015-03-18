#ifndef A2_AI_H
#define A2_AI_H

#include <stdio.h>
#include <assert.h> 
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <limits>
#include <math.h>
#include <limits.h>

#include <string>
#include <vector>
#include "board.h"
#include <math/point.hpp>

#include "except.h"

using namespace std;

typedef eecs467::Point<int> IntPoint;

static IntPoint TOPLEFT(0,0);
static IntPoint TOP(1,0);
static IntPoint TOPRIGHT(2,0);
static IntPoint LEFT(0,1);
static IntPoint CENTER(1,1);
static IntPoint RIGHT(2,1);
static IntPoint BOTLEFT(0,2);
static IntPoint BOT(1,2);
static IntPoint BOTRIGHT(2,2);

class a2_ai {


public: 

    Board board;
    char turn;

    bool check_end(Board& board_in);

    int opposite(int i);
    char opposite(char c);

    int search_line(Board& board_in, int x, int y, char a, char b);

    bool comp(IntPoint& a, IntPoint& b);

    bool in_center(char player, IntPoint& p);
    bool in_corner(char player, IntPoint& p);
    bool in_edge(char player, IntPoint& p);

    a2_ai();
    a2_ai(vector<vector<char>>& board_in, char turn_in);
    a2_ai(Board& board_in, char turn_in);
    a2_ai(string& s, char turn_in);
 
    bool set_turn(char turn_in);
    bool set_board(vector<vector<char>>& board_in);
    bool set_board(Board& board_in);
    bool update_cell(int x, int y, char t);

    void next_move(int& x, int& y);

    IntPoint p1_logic(char p1, char p2);
    IntPoint p2_logic(char p1, char p2);

    int look_ahead(int& x_in, int& y_in, char turn_in, Board& board_in);

    void print_state();
};

#endif // A2_AI_H
