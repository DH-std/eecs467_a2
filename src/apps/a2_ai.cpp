#include "a2_ai.h"

a2_ai::a2_ai() {
    turn = 'R';
    board.reset_board();
}

a2_ai::a2_ai(vector<vector<char>>& board_in, char turn_in) {
    assert(set_turn(turn_in));
    assert(board.set_board(board_in));
}

a2_ai::a2_ai(Board& board_in, char turn_in) {
    assert(set_turn(turn_in));
    assert(board.set_board(board_in.board));
}

a2_ai::a2_ai(string& s, char turn_in) {
    assert(set_turn(turn_in));
    assert(board.set_board(s));
}

void a2_ai::print_state() {
    board.print_board();
    //cout << "turn " << turn << endl;
}

bool a2_ai::set_turn(char turn_in) {
    if (turn_in == 'R') {
        turn = 'R';
    }
    else if (turn_in == 'G') {
        turn = 'G';
    }
    else {
        cout << "invalid turn: " << turn_in << endl;
        return false;
    }
    return true;
}

bool a2_ai::set_board(vector<vector<char>>& board_in) {
    return board.set_board(board_in);
}

bool a2_ai::set_board(Board& board_in) {
    return board.set_board(board_in.board);
}

bool a2_ai::update_cell(int x, int y, char t) {
    return board.update_cell(x, y, t);
}

void a2_ai::next_move(int& x, int& y) {
    IntPoint p;
    if (turn == 'R') {
        p = p1_logic('R', 'G');
    } 
    else if (turn == 'G') {
        p = p2_logic('R', 'G');
    }
    else {
        cout << "invalid turn" << turn << endl;
    }

    x = p.x;
    y = p.y;
}

bool a2_ai::check_end(Board& board_in) {
    for (int y = 0; y < 3; ++y) {
        if (board_in(0,y) + board_in(1,y) + board_in(2,y) == ('R' + 'R' + 'R') || 
                board_in(0,y) + board_in(1,y) + board_in(2,y) == ('G' + 'G' + 'G')) {
            return true;
        }
    }

    for (int x = 0; x < 3; ++x) {
        if (board_in(x,0) + board_in(x,1) + board_in(x,2) == ('R' + 'R' + 'R') || 
                board_in(x,0) + board_in(x,1) + board_in(x,2) == ('G' + 'G' + 'G')) {
            return true;
        }
    }
    
    if (board_in(0,0) + board_in(1,1) + board_in(2,2) == ('R' + 'R' + 'R') || 
            board_in(0,0) + board_in(1,1) + board_in(2,2) == ('G' + 'G' + 'G')) {
        return true;
    }

    if (board_in(0,2) + board_in(1,1) + board_in(2,0) == ('R' + 'R' + 'R') || 
            board_in(0,2) + board_in(1,1) + board_in(2,0) == ('G' + 'G' + 'G')) {
        return true;
    }
    return false;
}

int a2_ai::opposite(int i) {
    if (i == 0) {
        return 2;
    }
    else if (i == 2) {
        return 0;
    }
    else if (i == 1) {
        return 1;
    }
    else {
        cout << "unknown opposite of " << i << endl;
        return i;
    }
}

char a2_ai::opposite(char c) {
    if (c == 'R') {
        return 'G';
    }
    else if (c == 'G') {
        return 'R';
    }
    else {
        cout << "unknown opposite of " << c << endl;
        return c;
    }
}

int a2_ai::search_line(Board& board_in, int x, int y, char a, char b) {
    assert(x >= 0 && x < 3);
    assert(y >= 0 && y < 3);

    // if in center
    if (x == 1 && y == 1) {
        return ((board_in(TOPLEFT) + board_in(BOTRIGHT)) == a + b) + 
                ((board_in(TOP) + board_in(BOT)) == a + b ) +
                ((board_in(TOPRIGHT) + board_in(BOTLEFT)) == a + b) +
                ((board_in(RIGHT) + board_in(LEFT)) == a + b);

    }
    // if in corner, at even cell 0,0 0,2 2,0 2,2
    else if ( (x + y) % 2 == 0 ) {
        int op_x = opposite(x);
        int op_y = opposite(y);
        return ((board_in(x, 1) + board_in(x, op_y)) == a + b) + 
                ((board_in(op_x, y) + board_in(1, y)) == a + b) +
                ((board_in(CENTER) + board_in(op_x, op_y)) == a + b);
    }
    // if in edge, at odd cell, 0,1 1,0 2,1 1,2
    else if ( (x + y) % 2 == 1 ) {
        int op_x = opposite(x);
        int op_y = opposite(y);

        // if in top or botom edge
        if (x == 1) {
            return ((board_in(0, y) + board_in(2, y)) == a + b) +
                    ((board_in(CENTER) + board_in(x, op_y)) == a + b );
        } 
        // if in left or right edge
        else if (y == 1) {
            return ((board_in(x, 0) + board_in(x, 2)) == a + b) + 
                    ((board_in(CENTER) + board_in(op_x, y)) == a + b);
        }
        else {
            cout << "coordinate not in edge search line " << x << "," << y << endl;
            return -1;
        }
    }
    else {
        cout << "impossible entry search_line" << endl;
        return -1;
    }
}

int a2_ai::look_ahead(int& x_in, int& y_in, char turn_in, Board& board_in) {

    // cout << "look ahead turn " << turn_in << endl;

    if (check_end(board_in)) {
        cout << "look ahead on a finished board" << endl;
        
// board_in.print_board();
// cout << endl;

        x_in = -1;
        y_in = -1;
        return 0;
    }

    if (board_in.board_full()) {
        x_in = -1;
        y_in = -1;
        return 0;
    }

    char op_turn = opposite(turn_in);

    int score = 0;
    int best_score = INT_MIN;
    int best_x = -1, best_y = -1;
    // bool skip = false;

    // fast search if can win
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (board(x,y) == '.') {
                score = 0;

                // check if can win
                int num_win = search_line(board_in, x, y, turn_in, turn_in);
                if (num_win >= 1) {
//board_in.print_board();
cout << ">>>>>>>>>>>>>>fast found winner " << x << "," << y << " turn " << turn_in << endl << endl;

                    score = 30000;
                    x_in = x;
                    y_in = y;

                    best_x = x;
                    best_y = y;
                    best_score = score;
                    return score;
                }
            }
        }
    }

     // fast search if can block
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (board(x,y) == '.') {
                score = 0;               

                // check if can block oponent winning
                int num_block = search_line(board_in, x, y, op_turn, op_turn);
                if (num_block >= 1) {
                    score += num_block*900;

// board_in.print_board();
cout << ">>>>>>>>>>>>>>fast force block " << x << "," << y << " turn " << turn_in << endl << endl;

                    board.update_cell(x, y, turn_in);

                    score -= look_ahead(x_in, y_in, op_turn, board_in);

                    board.update_cell(x, y, '.');

                    x_in = x;
                    y_in = y;

                    best_x = x;
                    best_y = y;
                    best_score = score;
                    // skip = true;
                    return score;
                }

            }
        }
    }

     // fast search if can 2 way
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            if (board(x,y) == '.') {
                score = 0;   
                
                // check if can create 2 in same line
                int num_2_win = search_line(board_in, x, y, turn_in, '.');
                // if (num_2_win == 1) {
                //     score += 1000;
                // }
                // else 
                if (num_2_win > 1) {
                    score += num_2_win*2000;

                    x_in = x;
                    y_in = y;

                    best_x = x;
                    best_y = y;
                    best_score = score;
                    // skip = true;

cout << ">>>>>>>>>>>>>>fast 2 way" << endl;

                    return score;
                }

                // // check if can block opponent in same line
                // int num_2_block = search_line(board_in, x, y, op_turn, '.');
                // // if (num_2_block == 1) {
                // //     score += 900;
                // // }
                // // else 
                // if (num_2_block > 1) {
                //     score += num_2_block*9000;
                //     x_in = x;
                //     y_in = y;
                //     return score;
                // }
                
            }
        }
    }
 
    // look ahead check
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {

            if (board(x,y) == '.') {
// cout << "look ahead x " << x << " y " << y << " turn " << turn_in << endl;
// board_in.print_board();
// cout << endl;

                score = 0;

                // prioritize middle over corner over edge
                if (x == 1 && y == 1) {
                    score += 5;
                }
                // if in corner, at even cell 0,0 0,2 2,0 2,2
                else if ( (x + y)%2 == 0 ) {
                    score += 1;
                }

/*
                // check if can win
                int num_win = search_line(board_in, x, y, turn_in, turn_in);
                if (num_win >= 1) {
                    cout << ">>>>>>>>>>>>>>should never come here, found winner " << x << "," << y << " turn " << turn_in << endl << endl;

                    score += num_win*100000;
                    x_in = x;
                    y_in = y;
                    return score;
                }
                

                // check if can block oponent winning
                int num_block = search_line(board_in, x, y, op_turn, op_turn);
                if (num_block >= 1) {
                    score += num_block*900;

                    board_in.print_board();
                    cout << ">>>>>>>>>>>>>>fast force block " << x << "," << y << " turn " << turn_in << endl << endl;

                    board.update_cell(x, y, turn_in);

                    score -= look_ahead(x_in, y_in, op_turn, board_in);

                    board.update_cell(x, y, '.');

                    x_in = x;
                    y_in = y;
                    best_score = score;
                    // skip = true;
                    goto finish;
                }
*/

                /*
                // check if can create 2 in same line
                int num_2_win = search_line(board_in, x, y, turn_in, '.');
                if (num_2_win == 1) {
                    score += 1000;
                }
                else if (num_2_win > 1) {
                    score += num_2_win*10000;
                    x_in = x;
                    y_in = y;
                    return score;
                }

                // check if can block opponent in same line
                int num_2_block = search_line(board_in, x, y, op_turn, '.');
                if (num_2_block == 1) {
                    score += 900;
                }
                else if (num_2_win > 1) {
                    score += num_2_block*9000;
                    x_in = x;
                    y_in = y;
                    return score;
                }
                */

                int num_2_win = search_line(board_in, x, y, turn_in, '.');
                if (num_2_win > 0) {
                    score += num_2_win*1000;
                }

                int num_2_block = search_line(board_in, x, y, op_turn, '.');
                if (num_2_block > 0) {
                    score += num_2_block*900;
                }

                board.update_cell(x, y, turn_in);

                score -= look_ahead(x_in, y_in, op_turn, board_in);

                board.update_cell(x, y, '.');

// cout << " score " << score << " best_score " << best_score << endl;

                if (score > best_score) {
                    best_score = score;
                    best_x = x;
                    best_y = y;
                }
            }
        }
    }

// finish:

    if (best_x == -1 || best_y == -1) {
        cout << "error best_x or best_y equal -1 " << best_x << "," << best_y << " turn " << turn_in << endl;
        board_in.print_board();
        cout << endl;
    }
    if (best_score == INT_MIN) {
        cout << "error best_score equal INT_MIN" << endl;
    }

    x_in = best_x;
    y_in = best_y;

    return best_score;
}

IntPoint a2_ai::p1_logic(char p1, char p2) {
    int num_p1 = 0;
    int num_p2 = 0;

// cout << "p1 " << p1 << " p2 " << p2 << endl;

    if (p1 == 'R') {
        num_p1 = board.num_R;
        num_p2 = board.num_G;
    }
    else if (p1 == 'G') {
        num_p1 = board.num_G;
        num_p2 = board.num_R;
    }

    // empty board
    if (num_p1 == 0 && num_p2 == 0) {
        // put in a corner
        return TOPLEFT;
    }
    // other player start first
    else if (num_p1 == 0 && num_p2 == 1) {
        return p2_logic(p2, p1);
    }
    // get to go twice
    else if (num_p1 == 1 && num_p2 == 0){
        // put in center
        return CENTER;
    }
    // p1 second turn
    else if (num_p1 == 1 && num_p2 == 1) {
        IntPoint p1_coord;
        IntPoint p2_coord;
        vector<IntPoint> points;

        // p1 in corner
        if (in_corner(p1, p1_coord)) {
// cout << "p1 in corner" << endl;
// board.print_board();
// cout << endl;

            IntPoint op_corner = IntPoint(opposite(p1_coord.x), opposite(p1_coord.y));
            IntPoint ad_corner1 = IntPoint(opposite(p1_coord.x), p1_coord.y);
            IntPoint ad_corner2 = IntPoint(p1_coord.x, opposite(p1_coord.y));

            // p2 in center
            if (in_center(p2, p2_coord)) {
                // put in opposite corner 
                return op_corner;
            }
            // p2 in corner
            else if (in_corner(p2, p2_coord)) {
                // put in any free corner

                // if p2 in opposite corner
                if (comp(op_corner, p2_coord)) {
                    // put in adjacent corner
                    return ad_corner1;
                }
                else {
                    return op_corner;
                }
                
                // // get opposite corner
                // opposite_corner(p1_coord, points);
                // // if p2 in the opposite corner
                // if (comp(points.back(), p2_coord)) {
                //     // put in adjacent corner
                //     adjacent_corner(p1_coord, points);
                // }
                // else {
                //     // put in opposite corner
                //     // coord already at the back of points
                // }
                // return points.back();
            }
            // p2 in edge
            else if (in_edge(p2, p2_coord)) {
                // put in adjacent corner on side opposite of p2
                if (ad_corner1.x == p2_coord.x || ad_corner1.y == p2_coord.y) {
                    return ad_corner2;
                }
                else {
                    return ad_corner1;
                }

                // adjacent_corner(p1_coord, points);
                // if (points.back().x == p2_coord.x || points.back().y == p2_coord.y) {
                //     points.pop_back();
                // }
                // return points.back();
            }
            else {
                board.print_board();
                cout << endl;
                throw MyException("p1_logic some position detection error, p2 not in center, corner, edge");
            }              
        }
        // if p1 in center
        else if (in_center(p1, p1_coord)) {
            if (in_corner(p2, p2_coord)) {
                // put in corner oposite of p2
                return IntPoint(opposite(p2_coord.x),opposite(p2_coord.y));

                // opposite_corner(p2_coord, points);
                // return points.back();
            }
            else if (in_edge(p2, p2_coord)) {
                // put in opposite corner of p2

                if (p2_coord.x == 1) {
                    return IntPoint(0,opposite(p2_coord.y));
                }
                else if (p2_coord.y == 1) {
                    return IntPoint(opposite(p2_coord.x),0);
                }
                else {
                    board.print_board();
                    cout << endl;
                    throw MyException("p1_logic p2 coordinate not in edge");
                }
                
                // opposite_corner(p2_coord, points);
                // return points.back();
            }
            else {
                board.print_board();
                cout << endl;
                throw MyException("p1_logic some position detection error, p2 in center, p2 not in corner, edge");
            }
        }
        // if p1 in edge
        else if (in_edge(p1, p1_coord)) {
            IntPoint ad_edge1;
            IntPoint ad_edge2;
            IntPoint op_edge;
            if (p1_coord.x == 1) {
                ad_edge1 = LEFT;
                ad_edge2 = RIGHT;
                op_edge = IntPoint(p1_coord.x, opposite(p1_coord.y));
            }
            else if (p1_coord.y == 1) {
                ad_edge1 = TOP;
                ad_edge2 = BOT;
                op_edge = IntPoint(opposite(p1_coord.x), p1_coord.y);
            }
            else {
                board.print_board();
                cout << endl;
                throw MyException("p1_logic p1 coordinate not in edge");
            }    

            // if p2 in center
            if (in_center(p2, p2_coord)) {
                // put in opposite corner

                if (p1_coord.x == 1) {
                    return IntPoint(0,opposite(p2_coord.y));
                }
                else if (p1_coord.y == 1) {
                    return IntPoint(opposite(p2_coord.x),0);
                }
                else {
                    board.print_board();
                    cout << endl;
                    throw MyException("p1_logic p1 coordinate not in edge");
                }
                // opposite_corner(p1_coord, points);
                // return points.back();
            }
            // if p2 in corner
            else if (in_corner(p2, p2_coord)) {
                // if p2 in adjacent corner
                if (p1_coord.x == p2_coord.x || p1_coord.y == p2_coord.y) {
                    // put in adjacent edge near p2
                    if (ad_edge1.x == p2_coord.x || ad_edge1.y == p2_coord.y) {
                        return ad_edge1;
                    }
                    else {
                        return ad_edge2;
                    }

                    // return CENTER;
                }
                // if p2 in the opposite corner
                else {
                    // put in adjacent edge far from p2
                    if (ad_edge1.x == p2_coord.x || ad_edge1.y == p2_coord.y) {
                        return ad_edge2;
                    }
                    else {
                        return ad_edge1;
                    }
                }
            }
            // if p2 in edge
            else if (in_edge(p2, p2_coord)) {
                // if p2 in opposite edge
                if (comp(p2_coord, op_edge)) {
                    // put in any adjacent edge
                    return ad_edge1;
                }
                // p2 in adjacent edge opposite of p2
                else {
                    return IntPoint(opposite(p2_coord.x), opposite(p2_coord.y));
                }
            }
            else {
                board.print_board();
                cout << endl;
                throw MyException("p1_logic some position detection error, p2 not in center, corner, edge");
            }
        }
        else {
            board.print_board();
            cout << endl;
            throw MyException("p1_logic some position detection error, p1 not in center, corner, edge");
        }
    }
    cout<< "look ahead" << endl;
    
    IntPoint p;
    look_ahead(p.x, p.y, turn, board);
    return p;
}

IntPoint a2_ai::p2_logic(char p1, char p2) {
    int num_p1 = 0;
    int num_p2 = 0;

    if (p1 == 'R') {
        num_p1 = board.num_R;
        num_p2 = board.num_G;
    }
    else if (p1 == 'G') {
        num_p1 = board.num_G;
        num_p2 = board.num_R;
    }

    if (num_p1 == 0) {
        return p1_logic(p2, p1); 
    }
    else if (num_p1 == 1 && num_p2 == 0) {
        IntPoint p;
        if (in_corner(p1, p)) {
            // put in center
            return CENTER;
        }
        else if (in_center(p1, p)) {
            // put in corner
            return TOPLEFT;
        }
        else if (in_edge(p1, p)) {
            // put in center
            return CENTER;
        }
        else {
            throw MyException("p2 some position detection error");
        }
    }
    else if (num_p1 == 1 && num_p2 == 1) {
        return p1_logic(p2, p1);
    }

    // test these sepcial case
    // .R.
    // R..
    // ..G

    // R..
    // .G.
    // ..R

    // R..
    // ..G
    // .R.

    // else if (num_p1 == 2 && num_p2 == 1) {
    //     IntPoint p2_coord;
    //     if (in_center(p2)) {
    //         if ((board(TOPLEFT) == p1 && board(BOTRIGHT) == p1) ||
    //                 (board(TOPRIGHT) == p1 && board(BOTLEFT) == p1) ) {
    //             return LEFT;
    //         }
    //     }
    //     else if (p2_coord = in_corner(p2)) {
    //         if (p2_coord.x-1 == 1) {
    //             if (p2_coord.y-1 == 1)) {

    //             }
    //             else {
    //                 p2_coord.y+1 == 1
    //             }

    //         }
    //         else {
    //             p2_coord.x+1
    //             if (p2_coord.y-1 == 1)) {

    //             }
    //             else {
    //                 p2_coord.y+1 == 1
    //             }
    //         }
    //     }
    // }

    IntPoint p;
    look_ahead(p.x, p.y, turn, board);
    return p;
}

bool a2_ai::comp(IntPoint& a, IntPoint& b) {
    return (a.x == b.x && a.y == b.y);
}

bool a2_ai::in_center(char player, IntPoint& p) {
    if (board(1, 1) == player) {
        p = IntPoint(1,1);
        return true;
    }
    else {
        return false;
    }
    return false;
}

bool a2_ai::in_corner(char player, IntPoint& p) {
    if (board(0,0) == player) {
        p = IntPoint(0,0);
        return true;
    }
    else if (board(0,2) == player) {
        p =  IntPoint(0,2);
        return true;
    }
    else if (board(2,0) == player) {
        p =  IntPoint(2,0);
        return true;
    }
    else if (board(2, 2) == player) {
        p =  IntPoint(2,2);
        return true;
    }
    else {
        return false;
    }
}

bool a2_ai::in_edge(char player, IntPoint& p) {
    if (board(1,0) == player) {
        p =  IntPoint(1,0);
        return true;
    }
    else if (board(0,1) == player) {
        p =  IntPoint(0,1);
        return true;
    }
    else if (board(2,1) == player) {
        p =  IntPoint(2,1);
        return true;
    }
    else if (board(1, 2) == player) {
        p =  IntPoint(1,2);
        return true;
    }
    else {
        return false;
    }
}


// old code
/*
void opposite_corner(IntPoint p, vector<IntPoint>& points) {
    if (p.y == 0) {
        if (p.x == 0) {
            points.push_back(BOTRIGHT);
        }
        else if (p.x == 1) {
            points.push_back(BOTRIGHT);
            points.push_back(BOTLEFT);
        }
        else if (p.x == 2) {
            points.push_back(BOTLEFT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 1) {
        if (p.x == 0) {
            points.push_back(TOPRIGHT);
            points.push_back(BOTRIGHT);
        }
        else if (p.x == 1) {
            points.push_back(TOPLEFT);
            points.push_back(TOPRIGHT);
            points.push_back(BOTRIGHT);
            points.push_back(BOTLEFT);
        }
        else if (p.x == 2) {
            points.push_back(TOPLEFT);
            points.push_back(BOTLEFT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 2) {
        if (p.x == 0) {
            points.push_back(TOPRIGHT);
        }
        else if (p.x == 1) {
            points.push_back(TOPRIGHT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 2) {
            points.push_back(TOPLEFT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else {
        throw string("Invalid coordinate " << p.x << "," << p.y);
    }
}
void adjacent_corner(IntPoint p, vector<IntPoint>& points) {
    if (p.y == 0) {
        if (p.x == 0) {
            points.push_back(BOTLEFT);
            points.push_back(TOPRIGHT);
        }
        else if (p.x == 1) {
            points.push_back(TOPRIGHT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 2) {
            points.push_back(TOPLEFT);
            points.push_back(BOTRIGHT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 1) {
        if (p.x == 0) {
            points.push_back(BOTLEFT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 1) {
            points.push_back(BOTLEFT);
            points.push_back(BOTRIGHT);
            points.push_back(TOPRIGHT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 2) {
            points.push_back(TOPRIGHT);
            points.push_back(BOTRIGHT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 2) {
        if (p.x == 0) {
            points.push_back(BOTRIGHT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 1) {
            points.push_back(TOPRIGHT);
            points.push_back(TOPLEFT);
        }
        else if (p.x == 2) {
            points.push_back(BOTLEFT);
            points.push_back(TOPRIGHT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else {
        throw string("Invalid coordinate " << p.x << "," << p.y);
    }
}

IntPoint opposite_edge(IntPoint p) {
    if (p.y == 0) {
        if (p.x == 0) {
            points.push_back(BOT);
            points.push_back(RIGHT);
        }
        else if (p.x == 1) {
            points.push_back(BOT);
        }
        else if (p.x == 2) {
            points.push_back(LEFT);
            points.push_back(BOT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 1) {
        if (p.x == 0) {
            points.push_back(RIGHT);
        }
        else if (p.x == 1) {
            points.push_back(LEFT);
            points.push_back(BOT);
            points.push_back(RIGHT);
            points.push_back(TOP);
        }
        else if (p.x == 2) {
            points.push_back(LEFT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 2) {
        if (p.x == 0) {
            points.push_back(RIGHT);
            points.push_back(TOP);
        }
        else if (p.x == 1) {
            points.push_back(TOP);
        }
        else if (p.x == 2) {
            points.push_back(LEFT);
            points.push_back(TOP);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else {
        throw string("Invalid coordinate " << p.x << "," << p.y);
    }
}
IntPoint adjacent_edge(IntPoint p) {
    if (p.y == 0) {
        if (p.x == 0) {
            points.push_back(LEFT);
            pointwin_coords.push_back(TOP);
        }
        else if (p.x == 1) {
            points.push_back(LEFT);
            points.push_back(RIGHT);
        }
        else if (p.x == 2) {
            points.push_back(RIGHT);
            points.push_back(TOP);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 1) {
        if (p.x == 0) {
            points.push_back(TOP);
        }
        else if (p.x == 1) {
            points.push_back(LEFT);
            points.push_back(BOT);
            points.push_back(RIGHT);
            points.push_back(TOP);
        }
        else if (p.x == 2) {
            points.push_back(BOT);
            points.push_back(TOP);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else if (p.y == 2) {
        if (p.x == 0) {
            points.push_back(LEFT);
            points.push_back(BOT);
        }
        else if (p.x == 1) {
            points.push_back(LEFT);
            points.push_back(RIGHT);
        }
        else if (p.x == 2) {
            points.push_back(BOT);
            points.push_back(RIGHT);
        }
        else {
            throw string("Invalid coordinate " << p.x << "," << p.y);
        }
    }
    else {
        throw string("Invalid coordinate " << p.x << "," << p.y);
    }
}

void push_empty_coord(Board& board_in, char turn_in, vector<IntPoint>& coord, int x1, int y1, int x2, int y2, int x3, int y3) {

    if (board_in(x1, y1) + board_in(x2, y2) + board_in(x3, y3) == (turn_in + turn_in + '.')) {
        if (board_in(x1, y1) == '.') {
            coord.push_back(IntPoint(x1, y1));
        }
        else if (board_in(x2, y2) == '.') {
            coord.push_back(IntPoint(x2, y2));
        }
        else if (board_in(x3, y3) == '.') {
            coord.push_back(IntPoint(x3, y3));
        }
        else {
            cout << "num win logic error" << endl;
        }
    }
}

void win_coord(Board& board_in, char turn_in, vector<IntPoint>& coord) {
    assert(turn_in == 'R' || turn_in == 'G');

    for (int r = 0; r < 3; ++r) {
        push_empty_coord(board_in, turn_in, coord, 0, r, 1, r, 2, r);
    }

    for (int c = 0; c < 3; ++c) {
        push_empty_coord(board_in, turn_in, coord, r, 0, r, 1, r, 2);
    }
    
    push_empty_coord(board_in, turn_in, coord, 0, 0, 1, 1, 2, 2);
    push_empty_coord(board_in, turn_in, coord, 2, 0, 1, 1, 0, 2);
}

void lose_coord(Board& board_in, char turn_in, vector<IntPoint>& coord) {
    assert(turn_in == 'R' || turn_in == 'G');
    if (turn_in == 'R') {
        win_coord(board_in, 'G', coord);
    }
    else if (turn_in == 'G') {
        win_coord(board_in, 'R', coord);
    }
    else {
        cout << "invalid turn_in " << turn_in << endl; 
    }

}
*/