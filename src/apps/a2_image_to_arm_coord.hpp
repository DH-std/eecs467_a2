#ifndef __a2_image_to_arm_coord_H__
#define __a2_image_to_arm_coord_H__

class image_to_arm(){

public:

	int r1x, r1y, r2x, r2y; //arm values for the center of the first and last square
	int H1, H2, V1, V2 , S1 , S2;  //HSV max and min values 
	int c1x, c1y, c2x, c2y;  //camera values for the center of the first and last square
	char [100] path_name;  //path name of camera image

	double theta; //positive means rotate camera frame clockwise
	double s; // scaling factor to apply to camera frame
	double tx; // translate value to add camera frame by
	double ty;

	image_to_arm();
	image_to_arm(char[100] path, int Hmin, int Hmax, int Vmin , int Vmax, int Smin, int Smax);
	void save_first_square(); //save r1x, r1y values, awaits human input
	void save_last_square();  //save r2x, r2y values, awaits human input

	void calibrate(int r1x, int r1y, int r2x, int r2y, int c1x, int c1y, int c2x , int c2y); //calibrate values for theta, s, tx and ty
	void translate(double x, double y, double *outx, double *outy); //give a set of camera values, translates to arm values

	void print();
}

#endif

