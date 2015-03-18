#ifndef A2_BOARD_STATE_H
#define A2_BOARD_STATE_H

#include <stdio.h>
#include <gtk/gtk.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <signal.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <limits>
#include <math.h>
#include <math/point.hpp>

#include <lcm/lcm.h>

#include "vx/vx.h"
#include "vx/vx_util.h"
#include "vx/vx_remote_display_source.h"
#include "vx/vxo_drawables.h"
#include "vx/gtk/vx_gtk_display_source.h"

#include "common/getopt.h"
#include "common/timestamp.h"
#include "common/zarray.h"

#include "imagesource/image_u32.h"
#include "imagesource/image_util.h"
#include "imagesource/image_source.h"
#include "imagesource/image_convert.h"

#include <string>
#include <vector>

#include "board.h"
#include "a2_blob_detector.hpp"

using namespace std;

// typedef eecs467::Point<double> DoublePoint;
typedef eecs467::Point<int> IntPoint;

class a2_board_state {
public:
    char* url;
    image_source_t *isrc;

    int board_left, board_right, board_top, board_bot;
    int pick_left, pick_right, pick_top, pick_bot;

    int block6_x, block1_x, block6_y, block1_y;

    double HSV_R[6];
    double HSV_G[6];
    double HSV_B[6];

    char turn;

    IntPoint cell_center[9];

    Board board; // the board
    vector<IntPoint> to_pick; // pixel of coordinate to pick

    image_u32_t* im_board;
    image_u32_t* im_pick;

    bool camera_inited;

    a2_board_state();
    a2_board_state(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in);
    a2_board_state(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in, char* url_in, image_source_t *isrc_in);


    IntPoint find_mid(IntPoint a, IntPoint b);
    IntPoint find_mid(int x1, int y1, int x2, int y2);
    void init(string mask_board, string mask_pick, 
                          string hsv_R, string hsv_G, string hsv_B, char turn_in);

    bool init_camera();
    void init(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in, char* url_in, image_source_t *isrc_in);

    void get_camera_frame();
    bool onBoard(IntPoint ball_in, IntPoint& board_coord);
    bool update_board(vector<region>& regList, char turn_in);
    void update_pick(vector<region>& regList);
    bool detect();
    void save_board();

};


#endif // A2_BOARD_STATE_H