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
#include "a2_blob_detector.hpp"
#include "a2_ai.h"
#include "a2_inverse_kinematics.h"
#include "a2_pick_place.h"

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




// being changed 
/*
image_u32_t* get_camera_frame() {

    state_t* state = global_state;

    image_u32_t* im = NULL;

    zarray_t *urls = image_source_enumerate();

    printf("Cameras:\n");
    for (int i = 0; i < zarray_size(urls); i++) {
        char *url;
        zarray_get(urls, i, &url);
        printf("  %3d: %s\n", i, url);
    }

    if (zarray_size(urls) == 0) {
        printf("No cameras found.\n");
        exit(-1);
    }
    zarray_get(urls, 0, &state->url);

    state->isrc = image_source_open(state->url);

    if (state->isrc == NULL) {
        printf("Unable to open device %s\n", state->url);
        exit(-1);
    }

    image_source_t *isrc = state->isrc;

    if (isrc->start(isrc)) {
        cout << "can't start image source" << endl;
        exit(-1);
    }

    image_source_data_t isdata;

    int res = isrc->get_frame(isrc, &isdata);
    if (!res) {
        im = image_convert_u32(&isdata);
    }
    else {
        isrc->stop(isrc);
        cout << "can't get frame " << res << endl;
        exit(-1);
    }

    isrc->release_frame(isrc, &isdata);
    return im;
}
*/




class state_t{
public:

	getopt_t *gopt;

    lcm::LCM lcm;		
    pthread_t lcm_handler;
    pthread_t broadcast_handler;
    image_u32_t *im1;
    image_u32_t *im2;
    double servo_pos[6];

    long int tim;
    bool red;
    int turn_num;

    blob_detector detector1;
    blob_detector detector2;
    image_to_arm im2arm;

    Board board;
    a2_pick_place pickplace;
    a2_ai ai;
    a2_inverse_kinematics inverse;

    ifstream myfile;

    double x;
    double y;
    double z;

    double base_height;
    double upper_arm;
    double lower_arm;
    double palm_length;
    double finger_length;



public:
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

	state_t(){
		//define parameters
	    base_height = 11.7; // unit = cm
	    upper_arm = 10;
	    lower_arm = 10;
	    palm_length = 10;
	    finger_length = 8.2;
	    turn_num = 0;
	    tim = 0;


	    //read in values needed to calibrate
	    double Hmin, Hmax, Smin, Smax, Vmin, Vmax;	
	    
 		myfile.open ("HSV_cal.txt");
 		myfile >> Hmin >> Hmax >> Smin >> Smax >> Vmin >> Vmax;
 		myfile.close();

 		im1 = get_camera_frame();

	    //calibrate arm/camera values	
	    im2arm = image_to_arm(im1,Hmin,Hmax,Smin,Smax,Vmin,Vmax);
	    //read in camera points
	    int n;
	    cin >> n;
	    im2arm.camera_points(n);

	    //move arm to first square
	    cin >> n;
	    if(n==1){
	    	radiansToXYZ(servo_pos, this);
	    	im2arm.save_first_square(x,y);
	    }
	    //move arm to second square
	    cin >> n;
	    if(n>=0){
	    	radiansToXYZ(servo_pos, this);
	    	im2arm.save_last_square(x,y);
	    }
	    im2arm.calibrate();

	    //initialize ai



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
            	radiansToXYZ(servo_pos, this);
            	//print out xyz coord of arm
            	cout << "x coord: " << x << ", y coord: " << y << ", z coord: "<< endl;
            //printf ("\n");
        }

    void tttHandler(const lcm::ReceiveBuffer *rbuf,
	                  const std::string& channel,
	                  const ttt_turn_t *msg)
		{
			//check if our turn
			if(((red == true) && (msg->turn%2 == 0)) or ((red == false) && (msg->turn%2 == 1)) ){
				double Hmin, Hmax, Smin, Smax, Vmin, Vmax;	
				turn_num = msg->turn;

				int R_turn = ceil ((double)(msg->turn/2.0));
				int G_turn = floor ((double)(msg->turn/2.0));

			//check board state
				//blob detector for our balls
				myfile.open ("HSV_our.txt");
		 		myfile >> Hmin >> Hmax >> Smin >> Smax >> Vmin >> Vmax;
		 		myfile.close();

		 		im1 = get_camera_frame();

		 		detector1 = blob_detector(im1, Hmin, Hmax, Smin, Smax, Vmin, Vmax);
		 		detector1.process();

				//blob detector for opponents
				myfile.open ("HSV_their.txt");
		 		myfile >> Hmin >> Hmax >> Smin >> Smax >> Vmin >> Vmax;
		 		myfile.close();
		 		detector2 = blob_detector(im1, Hmin, Hmax, Smin, Smax, Vmin, Vmax);
		 		detector2.process();

		 		//update board

			//call ai (determine next move)
				a2_ai();

			//pick up ball


			//place ball


			//done with turn, update turn num
				turn_num++;


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
	state->tim += 50000;
	turn_msg.utime = state->tim;
	turn_msg.turn = state->turn_num;
	if(state->red == true){
		state->lcm.publish("RED_TURN", &turn_msg);
	}
	else{
		state->lcm.publish("GREEN_TURN", &turn_msg);
	}

	usleep(50000); //20Hz

	return NULL;
}



int main(int argc, char *argv[]){

	state_t state;
	if(*argv[1] == 'r'){
		state.red = true;
	}
	else{state.red = false;}

	//create threads
    pthread_create(&state.lcm_handler,NULL,lcm_loop, &state);
    pthread_create(&state.broadcast_handler, NULL, broadcast_loop, &state);	

}
