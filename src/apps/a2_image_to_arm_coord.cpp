#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "a2_blob_detector.hpp"
#include <imagesource/image_u32.h>
#include "lcmtypes/dynamixel_status_list_t.h"
#include "lcmtypes/dynamixel_status_t.h"
#include "a2_image_to_arm_coord.hpp"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include "math/fasttrig.h"
#define PI 3.141

//#include "a2_inverse_kinematics.cpp"

using namespace std;

//convert camera frame to arm frame
void image_to_arm::calibrate(){
    fasttrig_init();

    double xyarm[] = {r1x, r2x ,r3x ,
                      r1y, r2y ,r3y ,
                      1,   1,   1};

    double xycam[] = {c1x, c2x ,c3x , 
                      c1y, c2y ,c3y , 
                      1,   1,   1};

    for (int i = 0; i < 9; ++i) {
        trans[i] = 0.;
    }

    double invrs[] = {0., 0., 0.,
                      0., 0., 0.,
                      0., 0., 0.};

     
    gsl_matrix_view Aarm = gsl_matrix_view_array(xyarm, 3, 3);
    gsl_matrix_view Acam = gsl_matrix_view_array(xycam, 3, 3);
    gsl_matrix_view Atrn = gsl_matrix_view_array(trans, 3, 3);
    gsl_matrix_view Ainv = gsl_matrix_view_array(invrs, 3, 3);

    int s;
    gsl_permutation * p = gsl_permutation_alloc (3);
    gsl_linalg_LU_decomp (&Acam.matrix, p, &s);
    gsl_linalg_LU_invert (&Acam.matrix, p, &Ainv.matrix);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, &Aarm.matrix, 
                   &Ainv.matrix, 0.0, &Atrn.matrix);
    gsl_permutation_free (p);


     ofstream HSV_file("cal");

    HSV_file << trans[0] << " " << trans[1] << " "
            << trans[2] << " " << trans[3] << " "
            << trans[4] << " " << trans[5] << " " << endl;;

    HSV_file.close();
}

void image_to_arm::translate(double x, double y, double *outx, double *outy){
    *outx = x*trans[0] + y*trans[1] + trans[2];
    *outy = x*trans[3] + y*trans[4] + trans[5];
}


void image_to_arm::save_first_square(int x, int y){
    r1x = x;
    r1y = y;
}

void image_to_arm::save_second_square(int x, int y){
    r2x = x;
    r2y = y;
}

void image_to_arm::save_last_square(int x, int y){
    r3x = x;
    r3y = y;
}

void image_to_arm::print(){
    
}

image_to_arm::image_to_arm(){

}

void image_to_arm::camera_points(int n){
    //n is the number of the top left corner square
    detect.process();

    cout << "detect regNum " << detect.regNum << endl;

    if (detect.regNum != 5) {
        cout << "#####ERROR, can't detect 4 blue squares" << endl;
    }

    c1x = detect.regList[n].centerX ;
    c1y = detect.regList[n].centerY ;

    cout << "past first corner" << endl; 

    n+=1;
    if(n > 3){
        n-=4;
    } 
    cout << "second n value:" << n;
    //cout << detect->regNum << endl;

    c2x = detect.regList[n].centerX ;
    c2y = detect.regList[n].centerY ;
    cout << "past second corner" << endl; 

    n+=1;
    if(n > 3){
        n-=4;
    } 
    c3x = detect.regList[n].centerX ;
    c3y = detect.regList[n].centerY ;
    cout << "past third corner" << endl;


// string board_file = "im2arm";

// for (int x = -1; x <= 1; ++x) {
//     for (int y = -1; y <= 1; ++y) {
//         detect.image->buf[(detect.regList[0].centerY+y)*detect.image->stride+(detect.regList[0].centerX+x)] = 0xff000000;
//         detect.image->buf[(detect.regList[1].centerY+y)*detect.image->stride+(detect.regList[1].centerX+x)] = 0xff0000ff;
//         detect.image->buf[(detect.regList[2].centerY+y)*detect.image->stride+(detect.regList[2].centerX+x)] = 0xff00ff00;
//         detect.image->buf[(detect.regList[3].centerY+y)*detect.image->stride+(detect.regList[3].centerX+x)] = 0xffff0000;
//     }
// }

// if (!image_u32_write_pnm (detect.image, board_file.c_str())) {
//     cout << "board image saved to " + board_file << endl;
// }
// else {
//     cout << "error saving image to " + board_file << endl;
// }

}

image_to_arm::image_to_arm(string path, double Hmin, double Hmax, double Smin, double Smax, double Vmin , double Vmax){

    path_name = path;
    H1 = Hmin;
    H2 = Hmax;
    V1 = Vmin;
    V2 = Vmax;
    S1 = Smin;
    S2 = Smax;

    detect = blob_detector(path_name, H1,H2,S1,S2,V1,V2);
}

image_to_arm::image_to_arm(image_u32_t *im, double Hmin, double Hmax, double Smin, double Smax, double Vmin , double Vmax){


    H1 = Hmin;
    H2 = Hmax;
    V1 = Vmin;
    V2 = Vmax;
    S1 = Smin;
    S2 = Smax;

    image = im;
    detect = blob_detector(image, H1,H2,S1,S2,V1,V2);

cout << "image to arm inited" << endl;

}
