#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "a2_blob_detector.hpp"
#include "a2_blob_detector.cpp"
#include <imagesource/image_u32.h>
#include "lcmtypes/dynamixel_status_list_t.h"
#include "lcmtypes/dynamixel_status_t.h"
#include "a2_image_to_arm_coord.hpp"
#include <stdio.h>

#define PI 3.141

//#include "a2_inverse_kinematics.cpp"

using namespace std;

//convert camera frame to arm frame
void image_to_arm::calibrate(){
	double robot_center_x;
	double robot_center_y;
	double camera_center_x;
	double camera_center_y;

//calculate center/translate value
	robot_center_x = (r1x+r2x)/2.0;
	robot_center_y = (r1y+r2y)/2.0;
	camera_center_x = (c1x+c2x)/2.0;
	camera_center_y = (c1y+c2y)/2.0;

	tx = robot_center_x - camera_center_x;
	ty = robot_center_y - camera_center_y;

	printf("robot center x: %f\n", robot_center_x );
	printf("robot_center_y: %f\n", robot_center_y );
	printf("camera_center_x: %f\n", camera_center_x );
	printf("camera_center_y: %f\n", camera_center_y );

	//calculate scale value
	double r_d,r_dx,r_dy;
	double c_d,c_dx,c_dy;

	r_dx = r1x-r2x;
	r_dy = r1y-r2y;
	r_d = pow(r_dx,2)+pow(r_dy,2);
	r_d = pow(r_d,0.5);

	c_dx = c1x-c2x;
	c_dy = c1y-c2y;
	c_d = pow(c_dx,2)+pow(c_dy,2);
	c_d = pow(c_d,0.5);

	s = r_d/c_d;

	//calculate rotation value
	//start by calculating camera value prior to rotation
	double c1_px;
	double c1_py;
	double c1_pdx;
	double c1_pdy;

	c1_px = c1x + tx;
	c1_py = c1y + ty;

	c1_pdx = c1_px - robot_center_x;
	c1_pdy = c1_py - robot_center_y;
	c1_pdx *= s;
	c1_px = robot_center_x + c1_pdx;
	c1_pdy *= s;
	c1_py = robot_center_y + c1_pdy;


	theta =  PI/4 + atan(c1_pdy / c1_pdx);
	theta*=(180.0/PI);

}

void image_to_arm::translate(double x, double y, double *outx, double *outy){
	*outx = s*x*cos(theta) - s*y*sin(theta) + tx;
	*outy = s*x*sin(theta) + s*y*cos(theta) + ty;
}


void image_to_arm::save_first_square(int x, int y){
	r1x = x;
	r1y = y;
}

void image_to_arm::save_last_square(int x, int y){
	r2x = x;
	r2y = y;
}

void image_to_arm::print(){
	
}

image_to_arm::image_to_arm(){

}

void image_to_arm::camera_points(int n){
	//n is the number of the top left corner square
	detect->process();
	detect->regList[n].centerX = c1x;
	detect->regList[n].centerY = c1y;

	n+=3;
	if(n > 3){
		n-=4;
	} 

	detect->regList[n].centerX = c2x;
	detect->regList[n].centerY = c2y;
}

image_to_arm::image_to_arm(string path, int Hmin, int Hmax, int Smin, int Smax, int Vmin , int Vmax){

	path_name = path;
	Hmin = H1;
	Hmax = H2;
	Vmin = V1;
	Vmax = V2;
	Smin = S1;
	Smax = S2;

	blob_detector temp(path_name, H1,H2,S1,S2,V1,V2);
	detect = &temp;
}

image_to_arm::image_to_arm(image_u32_t *im, int Hmin, int Hmax, int Smin, int Smax, int Vmin , int Vmax){


	Hmin = H1;
	Hmax = H2;
	Vmin = V1;
	Vmax = V2;
	Smin = S1;
	Smax = S2;

	image = im;
	blob_detector temp(image, H1,H2,S1,S2,V1,V2);
	detect = &temp;
}

int main(){

}