#include <string>
#include <iostream>
#include <deque>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <cstdlib>
#include <fstream>

#include "board.h"
#include "a2_image_to_arm_coord.hpp"
//#include "a2_blob_detector.hpp"
#include "a2_ai.h"
#include "a2_inverse_kinematics.h"
#include "a2_pick_place.h"
#include "a2_board_state.h"

#include <stdio.h>
#include <stdlib.h>
#include <lcm/lcm-cpp.hpp>
#include <vector>
#include <gsl/gsl_blas.h>

#include "lcmtypes/dynamixel_command_list_t.hpp"
#include "lcmtypes/dynamixel_command_t.hpp"
#include "lcmtypes/dynamixel_status_list_t.hpp"
#include "lcmtypes/dynamixel_status_t.hpp"
#include "lcmtypes/ttt_turn_t.hpp"

#include "common/getopt.h"


class state_t{
public:

    getopt_t *gopt;

    lcm::LCM lcm;       
    pthread_t lcm_handler;
    pthread_t broadcast_handler;
    image_u32_t *im;
    double servo_pos[6];

    long int tim;
    bool red;
    int my_turn_num, other_turn_num;

    blob_detector detector1;
    blob_detector detector2;
    image_to_arm im2arm;

    Board board;
    a2_pick_place pick_place;
    a2_ai ai;
    a2_board_state board_state;

    ifstream myfile;

    double x;
    double y;
    double z;

    double base_height;
    double upper_arm;
    double lower_arm;
    double palm_length;
    double finger_length;

    bool camera_inited;
    bool init_done;
    char *url;
    image_source_t *isrc;

    a2_inverse_kinematics ik;

    int board_left, board_right, board_top, board_bot;

    bool init_camera() {
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

    void radiansToXYZ(double pos[], state_t *state) {
        fasttrig_init();
        double d = state->palm_length+state->finger_length;
        double a_f2w[] = {fcos(pos[3]), -1.*fsin(pos[3]), 0.,  d*fsin(pos[3]),
                          fsin(pos[3]),     fcos(pos[3]), 0., -d*fcos(pos[3]),
                                    0.,               0., 1.,              0.,
                                    0.,               0., 0.,              1.};

        d = state->lower_arm;
        double a_w2e[] = {fcos(pos[2]), -1.*fsin(pos[2]), 0.,  d*fsin(pos[2]),
                          fsin(pos[2]),     fcos(pos[2]), 0., -d*fcos(pos[2]),
                                    0.,              0.,  1.,              0.,
                                    0.,              0.,  0.,              1.};

        d = state->upper_arm;
        double a_e2s[] = {fcos(pos[1]), -1.*fsin(pos[1]), 0.,  d*fsin(pos[1]),
                          fsin(pos[1]),     fcos(pos[1]), 0., -d*fcos(pos[1]),
                                    0.,               0., 1.,              0.,
                                    0.,               0., 0.,              1.};

        d = state->base_height;
        double a_s2b[] = {fcos(pos[0]),  0., -1.*fsin(pos[0]), 0.,
                          fsin(pos[0]),  0.,     fcos(pos[0]), 0.,
                                    0., -1.,               0.,  d,
                                    0.,  0.,               0., 1.};

        double pt_f[] = {0., 0., 0., 1.};
        double pt_b[] = {0., 0., 0., 0.};
        double temp[] = {0., 0., 0., 0.,
                         0., 0., 0., 0.,
                         0., 0., 0., 0.,
                         0., 0., 0., 0.};
        double tem2[] = {0., 0., 0., 0.,
                         0., 0., 0., 0.,
                         0., 0., 0., 0.,
                         0., 0., 0., 0.};
     
        gsl_matrix_view Afw = gsl_matrix_view_array(a_f2w, 4, 4);
        gsl_matrix_view Awe = gsl_matrix_view_array(a_w2e, 4, 4);
        gsl_matrix_view Aes = gsl_matrix_view_array(a_e2s, 4, 4);
        gsl_matrix_view Asb = gsl_matrix_view_array(a_s2b, 4, 4);
        gsl_matrix_view Pf  = gsl_matrix_view_array(pt_f,  4, 1);
        gsl_matrix_view Pb  = gsl_matrix_view_array(pt_b,  4, 1);
        gsl_matrix_view tmp = gsl_matrix_view_array(temp,  4, 4);
        gsl_matrix_view tm2 = gsl_matrix_view_array(tem2,  4, 4);

        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &Asb.matrix, 
                       &Aes.matrix, 0.0, &tmp.matrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tmp.matrix, 
                       &Awe.matrix, 0.0, &tm2.matrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tm2.matrix, 
                       &Afw.matrix, 0.0, &tmp.matrix);
        gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &tmp.matrix, 
                       &Pf.matrix, 0.0, &Pb.matrix);

        state->x = pt_b[0];
        state->y = pt_b[1];
        state->z = pt_b[2];

        //cout << pt_b[0] << " " << pt_b[1] << " " << pt_b[2] << " " << pt_b[3] << endl;
    }

    state_t(char p){
        if(p == 'r'){
            red = true;
        }
        else{red = false;}

        //define parameters
        base_height = 11.7; // unit = cm
        upper_arm = 10;
        lower_arm = 10;
        palm_length = 10;
        finger_length = 8.2;
        my_turn_num = 0;
        other_turn_num = 0;
        tim = 0;

        camera_inited = false;
        init_done = false;

    }
    void init() {
        //subscribe to lcm message
        lcm.subscribe("ARM_STATUS", &state_t::handleStatus, this);

        //if red true, broadcast on red, recieve Green
        if(red == true){
            lcm.subscribe("GREEN_TURN", &state_t::tttHandler, this);
        }
        //if Green, broadcast green, receive Red
        else{
            lcm.subscribe("RED_TURN", &state_t::tttHandler, this);
        }
cout << "lcm subed" << endl;

        // ik = a2_inverse_kinematics();
        ik.limp();
              //read in values needed to calibrate
        double Hmin, Hmax, Smin, Smax, Vmin, Vmax;  
        
        myfile.open ("hsv_B");
        myfile >> Hmin >> Hmax >> Smin >> Smax >> Vmin >> Vmax;
        myfile.close();

cout << "got inital params" << endl;
cout << Hmin << "," << Hmax << "," << Smin << endl;

        ifstream mask_b("mask_board");
        mask_b >> board_left >> board_right >> board_top >> board_bot;
    // cout << "mask: " << mask_left << "," << mask_right << "," << mask_top << "," << mask_bot << endl;
        mask_b.close();

        init_camera();
cout << "camera inited" << endl;

        im = nullptr;

        image_source_data_t isdata;

        int res = isrc->get_frame(isrc, &isdata);
        if (!res) {
            im = image_convert_u32(&isdata);
            for (int y = 0; y < im->height; ++y) {
                for (int x = 0; x < im->width; ++x) {
                    if (y < board_top || y > board_bot || x < board_left || x > board_right) {
                        im->buf[y*im->stride+x] = 0xff000000;
                    }
                }
            }
cout << "got camera image" << endl;
        }
        else {
            isrc->stop(isrc);
            cout << "can't get frame " << res << endl;
            exit(-1);
        }

        isrc->release_frame(isrc, &isdata);




blob_detector detect = blob_detector(im, Hmin,Hmax,Smin,Smax,Vmin,Vmax);
detect.process();

string board_file = "im2arm";


for (unsigned int i = 0; i < detect.regList.size(); ++i) {
    cout << detect.regList[i].centerX <<","<< detect.regList[i].centerY << endl;
}

cout << "blob detector in state ctor" << endl;
for (int x = -1; x <= 1; ++x) {
    for (int y = -1; y <= 1; ++y) {
        im->buf[(detect.regList[0].centerY+y)*im->stride+(detect.regList[0].centerX+x)] = 0xff000000;
        im->buf[(detect.regList[1].centerY+y)*im->stride+(detect.regList[1].centerX+x)] = 0xff0000ff;
        im->buf[(detect.regList[2].centerY+y)*im->stride+(detect.regList[2].centerX+x)] = 0xff00ff00;
        im->buf[(detect.regList[3].centerY+y)*im->stride+(detect.regList[3].centerX+x)] = 0xffff0000;
    }
}

if (!image_u32_write_pnm (im, board_file.c_str())) {
    cout << "board image saved to " + board_file << endl;
}
else {
    cout << "error saving image to " + board_file << endl;
}

        

        //calibrate arm/camera values   
        
        im2arm = image_to_arm(im,Hmin,Hmax,Smin,Smax,Vmin,Vmax);
        if(1){
            myfile.open ("cal");
            myfile >> im2arm.trans[0] >> im2arm.trans[1] >> im2arm.trans[2] >> im2arm.trans[3] >> im2arm.trans[4] >> im2arm.trans[5];
            myfile.close();
        }
        else{
            //read in camera points
            int n;

            cout << "input which corner: ";
            cin >> n;
            im2arm.camera_points(n);

            //move arm to first square
            cout << "move arm to first square and press any key ";
            cin >> n;

                cout << endl;
          
                radiansToXYZ(servo_pos, this);
                im2arm.save_first_square(x,y);

            //move arm to second square
            cout << "move arm to second square and press any key ";
            cin >> n;

                radiansToXYZ(servo_pos, this); 
                im2arm.save_second_square(x,y);
     
            cout << "move arm to third square and press any key ";
            cin >> n;

                radiansToXYZ(servo_pos, this); 
                im2arm.save_last_square(x,y);

            cout << "press anykey to continue ";
            cin >> n; 

            im2arm.calibrate();
cout << "arm calibrated" << endl;
        }
        if (red) {
            board_state.init("mask_board", "mask_pick", "hsv_R", "hsv_G", "hsv_B", 'R', url, isrc);
        }
        else {
            board_state.init("mask_board", "mask_pick", "hsv_R", "hsv_G", "hsv_B", 'G', url, isrc);
        }


cout << "board_state inited" << endl;

        pick_place.move_origin();

        init_done = true;
}

    ~state_t(){


    }

    void handleStatus(const lcm::ReceiveBuffer *rbuf,
                      const std::string& channel,
                      const dynamixel_status_list_t *msg)
        {
            assert(channel == "ARM_STATUS");
            for (int id = 0; id < msg->len; id++) {
                servo_pos[id] = msg->statuses[id].position_radians;
              //  printf ("[id %d]=%6.3f ",id, stat.position_radians);
            }
                //radiansToXYZ(servo_pos, this);
                //print out xyz coord of arm
                //cout << "x coord: " << x << ", y coord: " << y << ", z coord: " << z << endl;
            //printf ("\n");
        }

    bool run_ttt() {
        // int R_turn = ceil ((double)(msg->turn/2.0));
        // int G_turn = floor ((double)(msg->turn/2.0));

        //check board state
        //blob detector for our balls

    //update board
cout << "detecting board" << endl;
        board_state.detect();
        // board_state.save_board();
        board_state.board.print_board();


    //call ai (determine next move)
        ai.set_board(board_state.board);

        if (red) {
            ai.set_turn('R');
        }
        else {
            ai.set_turn('G');
        }

cout << "ai board and player set" << endl;
cout << "ai board" << endl;
        ai.board.print_board();

        int board_x, board_y;
        ai.next_move(board_x, board_y);

cout << "ai next move " << board_x << "," << board_y << endl;

        if (board_x != -1 && board_y != -1) {

            double arm_x, arm_y;


            // calculate arm coordinate to pick, place

        //pick up ball
            if (board_state.to_pick.size() > 0) {
                im2arm.translate(board_state.to_pick.back().x, board_state.to_pick.back().y, &arm_x, &arm_y);

                // arm_x *= 0.98;
                // arm_y *= 0.98;

cout << "going to pick ball at pixel " << board_state.to_pick.back().x <<"," <<board_state.to_pick.back().y << " at arm coord " << arm_x << "," << arm_y << endl;

                pick_place.pick(arm_x, arm_y); 

cout << "ball picked" << endl;
            }
            else {
                cout << "no ball to pick" << endl;
                return false;
            }

        //place ball
            im2arm.translate(board_state.cell_center[3*board_y+board_x].x, board_state.cell_center[3*board_y+board_x].y, &arm_x, &arm_y);
cout << "going to place ball at " << arm_x << "," << arm_y << endl;

            pick_place.place(arm_x, arm_y);

cout << "ball placed" << endl;

        //done with turn, update turn num
            my_turn_num++;
        } //if (board_x != -1 && board_y != -1)
        else {
            cout << "game ended!" << endl;
            return false;
        }
        return true;

    }

    void tttHandler(const lcm::ReceiveBuffer *rbuf,
                      const std::string& channel,
                      const ttt_turn_t *msg)
        {
            //check if our turn
            // cout << " tttHandler msg->turn " << msg->turn << " my_turn_num " << my_turn_num << endl;
            if (init_done && (((red == true) && (msg->turn == my_turn_num)) or ((red == false) && (msg->turn != my_turn_num))) ){
                other_turn_num = msg->turn;
                run_ttt();
            }
            //if not our turn do nothing

        }


};

static void* lcm_loop(void *input)
{
    state_t* state = (state_t*) input;
    while(1){
        state->lcm.handle();
    }
    return NULL;
}

//broadcast ttt_turn_t
void * broadcast_loop(void *user){
    state_t *state = (state_t *)user;
    ttt_turn_t turn_msg;
    while(1){
        state->tim += 50000;
        turn_msg.utime = state->tim;
        turn_msg.turn = state->my_turn_num;
        if(state->red == true){
            state->lcm.publish("RED_TURN", &turn_msg);
        }
        else{
            state->lcm.publish("GREEN_TURN", &turn_msg);
        }

        usleep(50000); //20Hz
    }
    return NULL;
}



int main(int argc, char *argv[]){

    if (argc != 2) {
        cout << "specify the player r or g" << endl;
        exit(0);
    }   

    state_t state(*argv[1]);


    //create threads
    pthread_create(&state.lcm_handler,NULL,lcm_loop, &state);
    pthread_create(&state.broadcast_handler, NULL, broadcast_loop, &state); 

    cout << " thread created " << endl;

        state.init();

    cout << "state inited" << endl;
    while(1) {
        // char t;
        // cout << "press any key to run" << endl;
        // cin >> t;
        // state.run_ttt();
        usleep(1000000);
    }

}
