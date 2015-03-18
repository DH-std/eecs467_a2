#include "a2_board_state.h"

a2_board_state::a2_board_state(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in, char* url_in, image_source_t *isrc_in) {
    init(mask_board, mask_pick, hsv_R, hsv_G, hsv_B, turn_in, url_in, isrc_in);
}

a2_board_state::a2_board_state(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in) {
    url = nullptr;
    isrc = nullptr;
    init(mask_board, mask_pick, hsv_R, hsv_G, hsv_B, turn_in);
}

IntPoint a2_board_state::find_mid(IntPoint a, IntPoint b) {
    return find_mid(a.x, a.y, b.x, b.y);
} 

IntPoint a2_board_state::find_mid(int x1, int y1, int x2, int y2) {
    return IntPoint(round((x1+x2)/2.0), round((y1+y2)/2.0));
} 

a2_board_state::a2_board_state(){

}

void a2_board_state::init(string mask_board, string mask_pick, string hsv_R, string hsv_G, string hsv_B, char turn_in, char* url_in, image_source_t *isrc_in) {
    url = url_in;
    isrc = isrc_in;
    init(mask_board, mask_pick, hsv_R, hsv_G, hsv_B, turn_in);
}


void a2_board_state::init(string mask_board, string mask_pick, 
                          string hsv_R, string hsv_G, string hsv_B, char turn_in) {
    assert(turn_in == 'R' || turn_in == 'G');
    turn = turn_in;

    ifstream mask_b(mask_board);
    ifstream mask_p(mask_pick);
    mask_b >> board_left >> board_right >> board_top >> board_bot;
    mask_p >> pick_left >> pick_right >> pick_top >> pick_bot;
// cout << "mask: " << mask_left << "," << mask_right << "," << mask_top << "," << mask_bot << endl;
    mask_b.close();
    mask_p.close();
    
    assert(board_left < board_right);
    assert(board_top < board_bot);

    ifstream hsvR(hsv_R);
    ifstream hsvG(hsv_G);
    ifstream hsvB(hsv_B);

    // double t; 
    // ifstream hsvB(hsv_B);
    for (int i = 0; i < 6; ++i) {
        // hsvR >> t;
        // HSV_R[i] = round(t);

        // hsvG >> t;
        // HSV_G[i] = round(t);
        
        hsvB >> HSV_B[i];
        hsvR >> HSV_R[i];
        hsvG >> HSV_G[i];
    }
    hsvR.close();
    hsvG.close();
    hsvB.close();

    for (int i = 0; i < 6; ++i) {
        cout << "HSV_R["<<i<<"]" << HSV_R[i] <<",";
    }
    cout << endl;
    for (int i = 0; i < 6; ++i) {
        cout << "HSV_G["<<i<<"]" << HSV_G[i] <<",";
    }
    cout << endl;
    for (int i = 0; i < 6; ++i) {
        cout << "HSV_B["<<i<<"]" << HSV_B[i] <<",";
    }
    cout << endl;

    im_board = nullptr;
    im_pick = nullptr;

    // blue blob detector
    // calculate center of cells
    if (url == nullptr || isrc == nullptr) {
        camera_inited = false;
    }
    else {
        camera_inited = true;
    }
    
    init_camera();

    get_camera_frame();

    blob_detector blue_board(im_board, HSV_B[0], HSV_B[1], HSV_B[2], HSV_B[3], HSV_B[4], HSV_B[5]);
cout << "done initing blue" << endl;

    blue_board.process();

cout << "done processing blue" << endl;


    for (unsigned int i = 0; i < blue_board.regList.size(); ++i) {
        for (int x = -2; x <= 2; ++x) {
            for (int y = -2; y <= 2; ++y) {
                im_board->buf[(blue_board.regList[i].centerY+y)*im_board->stride+(blue_board.regList[i].centerX+x)] = 0xff00ff00;
                im_pick->buf[(blue_board.regList[i].centerY+y)*im_pick->stride+(blue_board.regList[i].centerX+x)] = 0xff00ff00;
            }
        }
    }
    save_board();

    if (blue_board.regList.size() == 4) {
        IntPoint blue_coord0 = IntPoint(blue_board.regList[0].centerX, blue_board.regList[0].centerY);
        IntPoint blue_coord1 = IntPoint(blue_board.regList[1].centerX, blue_board.regList[1].centerY);
        IntPoint blue_coord2 = IntPoint(blue_board.regList[2].centerX, blue_board.regList[2].centerY);
        IntPoint blue_coord3 = IntPoint(blue_board.regList[3].centerX, blue_board.regList[3].centerY);

        // IntPoint top_blue_mid = find_mid(blue_coord0, blue_coord1);
        // IntPoint bot_blue_mid = find_mid(blue_coord2, blue_coord3);

        IntPoint t1 = find_mid(blue_coord0, blue_coord3);
        IntPoint t2 = find_mid(blue_coord1, blue_coord2);

        cell_center[4] = find_mid(t1, t2);

        // cell_center[1] = find_mid(cell_center[4], top_blue_mid);
        // cell_center[7] = find_mid(cell_center[4], bot_blue_mid);

        cell_center[0] = find_mid(cell_center[4], blue_coord0);
        cell_center[2] = find_mid(cell_center[4], blue_coord1);
        cell_center[6] = find_mid(cell_center[4], blue_coord2);
        cell_center[8] = find_mid(cell_center[4], blue_coord3);

        cell_center[1] = find_mid(cell_center[0], cell_center[2]);
        cell_center[3] = find_mid(cell_center[0], cell_center[6]);
        cell_center[5] = find_mid(cell_center[2], cell_center[8]);
        cell_center[7] = find_mid(cell_center[6], cell_center[8]);

        // IntPoint top_blue_left= find_mid(blue_coord0, top_blue_mid);
        // IntPoint top_blue_right = find_mid(blue_coord1, top_blue_mid);
        // IntPoint bot_blue_left= find_mid(blue_coord2, bot_blue_mid);
        // IntPoint bot_blue_right = find_mid(blue_coord3, bot_blue_mid);

        // cell_center[1] = find_mid(cell_center[4], top_blue_mid);
    }
    else {
        cout << "WARNING!!!!!!! >>>>>>> can't find all 4 corner, use back up to find cell center" << endl;
        block6_x = round((board_right-board_left)/5.0);
        block1_x = ceil((board_right-board_left)/30.0);
        block6_y = round((board_bot-board_top)/5.0);
        block1_y = ceil((board_bot-board_top)/30.0);

        int block3_x = round((board_bot-board_top)/10.0);
        int block3_y = round((board_bot-board_top)/10.0);

        for (int y = 0; y < 3; ++y) {
            for (int x = 0; x < 3; ++x) {
                cell_center[y*3+x] = IntPoint(board_left + block3_x + (x+1)*block6_x, board_top + block3_y + (y+1)*block6_y);
            }
        }

    }

/*
    for (int y = 0; y < 3; ++y) {
        for (int x = 0; x < 3; ++x) {
            cout << "cell[" << y*3+x << "]:" << cell_center[y*3+x].x << "," << cell_center[y*3+x].y << "|";
        }
    }
    cout << endl;
*/

    board.reset_board();
    to_pick.clear();
    
}


bool a2_board_state::init_camera() {
    if (!camera_inited) {

        zarray_t *urls = image_source_enumerate();

        printf("Cameras:\n");
        for (int i = 0; i < zarray_size(urls); i++) {
            char *url;
            zarray_get(urls, i, &url);
            // printf("  %3d: %s\n", i, url);
        }

        if (zarray_size(urls) == 0) {
            printf("No cameras found.\n");
            exit(-1);
        }
        zarray_get(urls, 0, &url);

        isrc = image_source_open(url);

        if (isrc == NULL) {
            printf("Unable to open device %s\n", url);
            exit(-1);
        }

        if (isrc->start(isrc)) {
            cout << "can't start image source" << endl;
            exit(-1);
        }
        camera_inited = true;
        return false;
    }
    else {
        cout << "camera already inited" << endl;
        return true;
    }

}

void a2_board_state::get_camera_frame() {

    if (im_board != nullptr) {
        image_u32_destroy(im_board);
        im_board = nullptr;
    }
    if (im_pick != nullptr) {
        image_u32_destroy(im_pick);
        im_pick = nullptr;
    }

    image_source_data_t isdata;

cout << " going to get isdata" << endl;

    int res = isrc->get_frame(isrc, &isdata);

cout << "got isrc" << endl;


    if (!res) {
        im_pick = image_convert_u32(&isdata);
        im_board = image_convert_u32(&isdata);
    }
    else {
        isrc->stop(isrc);
        cout << "can't get frame " << res << endl;
        exit(-1);
    }

cout << "going to release isrc" << endl;

    isrc->release_frame(isrc, &isdata);

    for (int y = 0; y < im_pick->height; ++y) {
        for (int x = 0; x < im_pick->width; ++x) {
            if (y < board_top || y > board_bot || x < board_left || x > board_right) {
                im_board->buf[y*im_board->stride+x] = 0xff000000;
            }
            if (y < pick_top || y > pick_bot || x < pick_left || x > pick_right) {
                im_pick->buf[y*im_pick->stride+x] = 0xff000000;
            }
        }
    }

    for (int i = 0; i < 9; ++i) {
        im_board->buf[cell_center[i].y*im_board->stride+cell_center[i].x] = 0xffffff00;
        im_pick->buf[cell_center[i].y*im_pick->stride+cell_center[i].x] = 0xffffff00;
    }


    // save_board();
    // cout << "init board saved" << endl;
}


bool a2_board_state::onBoard(IntPoint ball_in, IntPoint& board_coord) {
    int x = ball_in.x;
    int y = ball_in.y;

    double best_dist = 40;
    bool ret = true;
    for (int i = 0; i < 9; ++i) {
        double dist = sqrt( (cell_center[i].y-y)*(cell_center[i].y-y) + (cell_center[i].x-x)*(cell_center[i].x-x) );
        if (dist < best_dist && board(i%3,i/3) == '.') {
            best_dist = dist;
            board_coord.x = i%3;
            board_coord.y = i/3;
            ret = true;
        }
    }

    return ret;
    /*
    IntPoint board_coord_temp(-1,-1);

    if (x > board_left+block6_x+block1_x && x < board_left+2*block6_x-block1_x) {
        board_coord_temp.x = 0;
    }
    else if (x > board_left+2*block6_x+block1_x && x < board_left+3*block6_x-block1_x) {
        board_coord_temp.x = 1;
    }
    else if (x > board_left+3*block6_x+block1_x && x < board_left+4*block6_x-block1_x) {
        board_coord_temp.x = 2;
    }
    else {
        return false;
    }

    if (y > board_top+block6_y+block1_y && y < board_top+2*block6_y-block1_y) {
        board_coord_temp.y = 0;
    }
    else if (y > board_top+2*block6_y+block1_y && y < board_top+3*block6_y-block1_y) {
        board_coord_temp.y = 1;
    }
    else if (y > board_top+3*block6_y+block1_y && y < board_top+4*block6_y-block1_y) {
        board_coord_temp.y = 2;
    }
    else {
        return false;
    }

    if (board_coord_temp.x != -1 && board_coord_temp.y != -1) {
        board_coord.x = board_coord_temp.x;
        board_coord.y = board_coord_temp.y;
        return true;
    }
    else {
        return false;
    }
    */
}

bool a2_board_state::update_board(vector<region>& regList, char turn_in) {
    bool ret = true;
    for (unsigned int i = 0; i < regList.size(); ++i) {
        IntPoint board_coord(-1,-1);

        if (turn_in == 'R') {
            for (int x = -2; x <= 2; ++x) {
                for (int y = -2; y <= 2; ++y) {
                    im_board->buf[(regList[i].centerY+y)*im_board->stride+(regList[i].centerX+x)] = 0xff0000ff;
                    im_pick->buf[(regList[i].centerY+y)*im_pick->stride+(regList[i].centerX+x)] = 0xff0000ff;
                }
            }
        }
        else {
            for (int x = -2; x <= 2; ++x) {
                for (int y = -2; y <= 2; ++y) {
                    im_board->buf[(regList[i].centerY+y)*im_board->stride+(regList[i].centerX+x)] = 0xff00ff00;
                    im_pick->buf[(regList[i].centerY+y)*im_pick->stride+(regList[i].centerX+x)] = 0xff00ff00;
                }
            }
        }


        if (onBoard(IntPoint(regList[i].centerX, regList[i].centerY), board_coord)) {
            cout << turn_in << " detection at " << regList[i].centerX << "," << regList[i].centerY << " go to " << board_coord.x << "," << board_coord.y << endl;
            board.update_cell(board_coord.x, board_coord.y, turn_in);
        }
        else {
            cout << "####ERROR :: blob at (" << regList[i].centerX << "," << regList[i].centerY << ") is not on board" << endl;
            ret = false;
        }
    }
    return ret;
}

void a2_board_state::update_pick(vector<region>& regList) {
    to_pick.resize(regList.size());

    for (unsigned int i = 0; i < regList.size(); ++i) {
        to_pick[i].x = regList[i].centerX;
        to_pick[i].y = regList[i].centerY;

        for (int x = -2; x <= 2; ++x) {
            for (int y = -2; y <= 2; ++y) {
                im_board->buf[(regList[i].centerY+y)*im_board->stride+(regList[i].centerX+x)] = 0xffff00ff;
                im_pick->buf[(regList[i].centerY+y)*im_pick->stride+(regList[i].centerX+x)] = 0xffff00ff;
            }
        }
    }
}

bool a2_board_state::detect() {
    assert(turn == 'R' || turn == 'G');

    get_camera_frame(); // get masked image from cam

// cout << "initing red detector" << endl;
    blob_detector red_board(im_board, HSV_R[0], HSV_R[1], HSV_R[2], HSV_R[3], HSV_R[4], HSV_R[5]);
// cout << "initing green detector" << endl;
    blob_detector green_board(im_board, HSV_G[0], HSV_G[1], HSV_G[2], HSV_G[3], HSV_G[4], HSV_G[5]);
// cout << "initing pick detector" << endl;
    blob_detector* off_board;

    if (turn == 'R') {
        off_board = new blob_detector(im_pick, HSV_R[0], HSV_R[1], HSV_R[2], HSV_R[3], HSV_R[4], HSV_R[5]);
    }
    else {
        off_board = new blob_detector(im_pick, HSV_G[0], HSV_G[1], HSV_G[2], HSV_G[3], HSV_G[4], HSV_G[5]);
    }


// cout << "done init blob detector" << endl;

// cout << "processing red" << endl;
    red_board.process();
// cout << "processing green" << endl;
    green_board.process();
// cout << "processing off board" << endl;
    off_board->process();

// cout << "done process blob detector" << endl;

    bool ok_R = update_board(red_board.regList, 'R');
    bool ok_G = update_board(green_board.regList, 'G');
    update_pick(off_board->regList);

// cout << "got camera " << endl;
save_board();
// cout << "board saved" << endl;

    delete off_board;
    return ok_R && ok_G;
}


string board_file = "board.pmm";
string pick_file = "pick.pmm";

void a2_board_state::save_board() {

// cout << "im_board " << im_board << " im_pick " << im_pick << endl;

    if (!image_u32_write_pnm (im_board, board_file.c_str())) {
        cout << "board image saved to " + board_file << endl;
    }
    else {
        cout << "error saving image to " + board_file << endl;
    }

    if (!image_u32_write_pnm (im_pick, pick_file.c_str())) {
        cout << "pick image saved to " + pick_file << endl;
    }
    else {
        cout << "error saving image to " + pick_file << endl;
    }

}