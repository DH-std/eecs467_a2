#include "board.h"

Board::Board() { 
    reset_board();
}

Board::Board(vector<vector<char>>& board_in) { 
    reset_board();
    assert(set_board(board_in));
}

Board::Board(Board& board_in) {
    reset_board();
    assert(set_board(board_in.board));
}

Board::Board(string& s) {
    reset_board();
    assert(set_board(s));  
}

bool Board::set_board(vector<vector<char>>& board_in) {
    num_R = 0;
    num_G = 0;

    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            board[y][x] = '.';
        }
    }
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (!update_cell(x, y, board_in[y][x])) {
                return false;
            }
        }
    }
    return true;
}

bool Board::set_board(string& s) {
    if (s.size() != 9) {
        return false;
    }
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (!update_cell(x, y, s[y*3+x])) {
                return false;
            }
        }
    }   
    return true;
}

bool Board::reset_board() {
    num_R = 0;
    num_G = 0;
    board.resize(3, vector<char>(3,'.'));
    // for (int y = 0; y < 3; ++y) {
    //     for (int x = 0; x < 3; ++x) {
    //         board[y][x] = '.';
    //     }
    // }
    return true;
}

bool Board::update_cell(int x, int y, char t) {
    if (y < 0 || y > 2 || x < 0 || x > 2 || !(t == '.' || t == 'R' || t == 'G' )) {
        cout << "update cell invalid row, col, or input: " << y << "," << x << "," << t << endl;
        return false;
    }
    else {
        if (t == 'R') {
            if (board[y][x] == 'R') {
            }
            else if (board[y][x] == 'G') {
                num_R++;
                num_G--;
                board[y][x] = t;
            }
            else {
                num_R++;
                board[y][x] = t;
            }
        }
        else if (t == 'G') {
            if (board[y][x] == 'R') {
                num_R--;
                num_G++;
                board[y][x] = t;
            }
            else if (board[y][x] == 'G') {
            }
            else {
                num_G++;
                board[y][x] = t;
            }
        }
        else if (t == '.') {
            if (board[y][x] == 'R') {
                num_R--;
                board[y][x] = t;
            }
            else if (board[y][x] == 'G') {
                num_G--;
                board[y][x] = t;
            }
            else {
            }
        }
        else {
            cout << "invalid input: " << t << endl;
        }
        return true;
    }
}

char Board::operator()(int x, int y)
{
    assert(x >= 0 && x < 3);
    assert(y >= 0 && y < 3);
 
    return board[y][x];
}

char Board::operator()(int idx)
{
    assert(idx < 9);
 
    return board[idx/3][idx%3];
}

char Board::operator()(IntPoint& p)
{
    assert(p.x >= 0 && p.x < 3);
    assert(p.y >= 0 && p.y < 3);
 
    return board[p.y][p.x];
}

bool Board::board_full() {
    return (num_R + num_G == 9);
}

void Board::print_board() {
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            cout << board[y][x] << " ";
        }
        cout << endl;
    }
    //cout << "num_R " << num_R << " num_G " << num_G << endl;
}
