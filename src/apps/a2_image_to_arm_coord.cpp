#include <math.h>
#include <stdlib.h>
#include <imagesource/image_u32.h>
#include "lcmtypes/dynamixel_status_list_t.lcm"
#include "lcmtypes/dynamixel_status_t.lcm"
#define PI 3.141
using namespace std;

//convert camera frame to arm frame
void calibrate( int r1x, int r1y, int r2x, int r2y, int c1x, int c1y, int c2x , int c2y){
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

void translate(double x, double y, double *outx, double *outy){
	*outx = s*x*cos(theta) - s*y*sin(theta) + tx;
	*outy = s*x*sin(theta) + s*y*cos(theta) + ty;
}


void save_first_square(){
	int x;
	//top left corner (from camera's point of view)
	cin >> x; 
	//type 1 to 
	if(x==1){
		//inverse kinematic current servo 
		//save left corner value
		r1x = ;
		r1y = ;
	}
}

void save_last_square(){
	int x;
	//bottom right corner
	cin >> x;
	if(x==1){
		//inverse kinematic 
		//save left corner value
		r2x = ;
		r2y = ;
	}
}

void print(){
	
}

image_to_arm(){

}

image_to_arm(char[100] path, int Hmin, int Hmax, int Vmin , int Vmax, int Smin, int Smax){

	path_name = path;
	Hmin = H1;
	Hmax = H2;
	Vmin = V1;
	Vmax = V2;
	Smin = S1;
	Smax = S2;


}