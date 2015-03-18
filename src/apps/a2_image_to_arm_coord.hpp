#ifndef __a2_image_to_arm_coord_H__
#define __a2_image_to_arm_coord_H__
#include <string>
#include "a2_blob_detector.hpp"
using namespace std;
class image_to_arm{

public:

    double r1x, r1y, r2x, r2y,r3x,r3y; //arm values for the center of the first and last square
    double H1, H2, V1, V2 , S1 , S2;  //HSV max and min values 
    double c1x, c1y, c2x, c2y,c3x,c3y;  //camera values for the center of the first and last square
    string path_name;  //path name of camera image
    image_u32_t *image;

    double robot_center_x;
    double robot_center_y;
    double camera_center_x;
    double camera_center_y;

    double trans[9];

    double theta; //positive means rotate camera frame clockwise
    double s; // scaling factor to apply to camera frame
    double tx; // translate value to add camera frame by
    double ty;
    int num; //num of the top left corner
    blob_detector detect;

    image_to_arm();
    image_to_arm(string path, double Hmin, double Hmax, double Smin, double Smax, double Vmin , double Vmax);
    image_to_arm(image_u32_t *im, double Hmin, double Hmax, double Smin, double Smax, double Vmin ,double Vmax);
    void save_first_square(int x, int y); //save r1x, r1y values, awaits human input
    void save_second_square(int x, int y);
    void save_third_square(int x, int y);
    void save_last_square(int x, int y);  //save r2x, r2y values, awaits human input
    void camera_points(int n);

    void calibrate(); //calibrate values for theta, s, tx and ty
    void translate(double x, double y, double *outx, double *outy); //give a set of camera values, translates to arm values

    void print();
};
#endif

