#include <math.h>
#include <stdlib.h>
#include "a2_blob_detector.hpp"
#include <lcm/lcm-cpp.hpp>
#include <deque>
#include <vector>
#include <string>
#include <string.h>
#include <iostream>
#include <stdio.h>
#include "common/getopt.h"
#include <imagesource/image_u32.h>

#include <gtk/gtk.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
using namespace std;

#define minArea 150 //min area to be considered a region


void blob_detector::HSV(uint32_t val, double * H, double * S, double *V){

    //convert to rgba first

    //printf("val:0x%06x\n", val);
    int rold, gold, bold;

    rold = (val >> 0) & 0x000000ff;
    gold = (val >> 8) & 0x000000ff;
    bold = (val >> 16) & 0x000000ff;
    // val = val % (1<<24);
    // bold = val / (1<<16) ;
    // val = val % (1<<16);
    // gold = val / (1 << 8);
    // val = val % (1<<8);
    // rold = val;

    //printf("rold:%d\n", rold);
    //printf("gold:%d\n", gold);
    //printf("bold:%d\n", bold);

    double r,g,b;
    r = rold/255.0;
    g = gold/255.0;
    b = bold/255.0;

    //printf("r:%f\n", r);
    //printf("g:%f\n", g);
    //printf("b:%f\n", b);
    

    double cmax, cmin, delta;
    int col = 0;

    if(r >=  b &&  r >= g){
        cmax = r;
        col = 1;
    }
    else if( g >=r  && g >= b){
        cmax = g;
        col = 2;
    }
    else{
        cmax = b;
        col = 3;
    }

    if(r <=  b &&  r <= g){
        cmin = r;
    }
    else if( g <=r  && g <= b){
        cmin = g;
    }
    else{
        cmin = b;
    }

    delta = cmax - cmin;

    if(delta == 0){
        *H = 0;
    }
    else if(col == 1){
//cout << " kkkk " << H << endl;
//cout << " " << " g " << g << endl;
//cout << " b " << b << endl;
//cout << " delta " << delta << endl;

        *H = (g-b)/delta;

//cout << "H " << *H  << endl;
        *H = fmod(*H,6);
        *H *= 60;
    }
    else if(col == 2){
        *H = 60*(((b-r)/delta)+2);
    }
    else if(col == 3){
        *H = 60*(((r-g)/delta)+4);
    }

    if(cmax == 0){
        *S = 0;
    }
    else{
        *S = delta/cmax;
        *S *= 100;
    }
    *V = cmax;
    *V *= 100;
}   

static volatile image_u32_t *test;

bool blob_detector::checkpoint(int t, image_u32_t * im){
    double  S, V;
    double H;

    int curY = t/stride;
    int curX = t%stride;
    uint32_t val = (im->buf)[curY*stride+curX];
    test = im;
    HSV(val, &H, &S, &V);
    //cout << " HSV done" << endl;
    double Hval = H;
    double Sval = S;
    double Vval = V;
/*
    if (curX == 610 && curY == 290) {
        cout << "Y:" << curY << ",X:" << curX << ",H:" << Hval << ",S:" << Sval << ",V:" << Vval << endl;
        cout << "Hmin:" << Hmin << ",Hmax:" << Hmax << ",Smin:" << Smin << ",Smax:" << Smax << ",Vmin:" << Vmin << ",Vmax:" << Vmax << endl;

        if((Hval <= Hmax) && (Hval >= Hmin) ){
            if((Sval <= Smax) && (Sval >= Smin)){
                if((Vval <= Vmax) && (Vval >= Vmin)){
                    cout << "PASSED!!!!" << endl;
                }
            }
        }
    }
    */
    // printf("curY:%d\n", curY);
    // printf("curX:%d\n", curX);
    // printf("Hval:%d\n", Hval);
    // printf("Sval:%f\n", Sval);
    // printf("Vval:%f\n", Vval);
    if((Hval <= Hmax) && (Hval >= Hmin) ){
        if((Sval <= Smax) && (Sval >= Smin)){
            if((Vval <= Vmax) && (Vval >= Vmin)){
                return true;
            }
        }
    }


    return false;
}

void blob_detector::bucketfill(int curX, int curY, image_u32_t * im,
                     deque<int> *nextcoord, region * reg){

    //printf("y:%d, x:%d\n",curY , curX);
    //search surrounding neighbors
    for(int i = 0; i < 8; i++){
        int a,b;
        switch(i){
            case 0:
                a = -1;
                b = -1;
                break;
            case 1:
                a = 0;
                b = -1;
                break;
            case 2:
                a = 1;
                b = -1;
                break;
            case 3:
                a = -1;
                b = 0;
                break;
            case 4:
                a = 1;
                b = 0;
                break;
            case 5:
                a = -1;
                b = 1;
                break;
            case 6:
                a = 0;
                b = 1;
                break;
            case 7:
                a = 1;
                b = 1;
                break;
        }

        //printf("i:%d\n", i);
        if(( ((curX+a) < width) && ((curX+a) > 0) && ((curY+b) > 0) && ((curY+b) < height)) == true){
            if( vis[(curY + b)*stride + (curX+a)] == false && checkpoint((curY + b)*stride + (curX+a), im)){
                nextcoord->push_back((curY + b)*stride + (curX+a));
                vis[(curY + b)*stride + (curX+a)] = true;
            }
        }


    }

    //update region info
    reg->area += 1;
    reg->compX(curX);
    reg->compY(curY);


    //find next coord if deque not empty
    if(nextcoord->size()!= 0 ){
        int n = nextcoord->front();
        nextcoord->pop_front();
        curY = n/stride;
        curX = n%stride;
        //usleep(500000);
        bucketfill(curX, curY, im, nextcoord, reg);
    }

}

blob_detector::blob_detector(){
    regNum = 1;
    gopt = getopt_create ();
}

blob_detector::blob_detector(string path_name, double H1, double H2, double S1, double S2, double V1, double V2){
    
    gopt = getopt_create ();
    //initialize HSV values from task2
    Hmin = H1;
    Hmax = H2;
    Smin = S1;
    Smax = S2;
    Vmin = V1;
    Vmax = V2;

    regNum = 1;

    //load pnm image into image_u32_t
    char path[200];

    strcpy(path, path_name.c_str());
    image = image_u32_create_from_pnm(path);
    if (image == NULL) {
        printf("image %s unsuccesfully loaded\n", path);
        exit(1);
    }
    else{
    width = image->width;
    height = image->height;
    stride = image->stride;

    // printf("width:%d\n", width);
    // printf("height:%d\n", height);
    // printf("stride:%d\n", stride);

    vis.assign(width*height, 0);
    }
}

blob_detector::blob_detector(image_u32_t *im, double H1, double H2, double S1, double S2, double V1, double V2){

    Hmin = H1;
    Hmax = H2;
    Smin = S1;
    Smax = S2;
    Vmin = V1;
    Vmax = V2;

    regNum = 1;

    image = im;

    width = image->width;
    height = image->height;
    stride = image->stride;

     printf("width:%d\n", width);
     printf("height:%d\n", height);
     printf("stride:%d\n", stride);

    vis.assign(width*height, 0);
cout << "blob detector inited" << endl;
}

void blob_detector::process(){
    
    //looping to find start positions
    for(int y=1; y < height - 1; y++){
        for(int x=1; x < width -1; x++){

            if(vis[y*stride+x] == false && checkpoint(y*stride+x, image)){
                
                // printf("good\n");
                deque<int> nextcoord;
                region temp_reg(x, y, regNum);
                // printf("startx:%d\n", x);
                // printf("starty:%d\n", y);

                vis[y*stride+x] = true;
                // printf("good2\n");
                bucketfill(x, y, image , &nextcoord, &temp_reg );
                // printf("good3\n");
// cout << temp_reg.centerX << "," << temp_reg.centerY << " temp_reg.area " << temp_reg.area << endl;
                if(temp_reg.area >= minArea){
                    temp_reg.setCenter();
                     printf("centX:%d\n", temp_reg.centerX);
                     printf("centy:%d\n", temp_reg.centerY);
                    regList.push_back(temp_reg);
                    regNum++;
                        printf("reglist size: %d\n", regNum);
                }
                // printf("good4\n");

            }
            else{
                vis[y*stride+x] = true;

            }

        }
    }

}
// int main(){


//  string p = "/home/loren/Documents/Mich/eecs467a2/src/pics/corner_image_0.ppm";

//  blob_detector b1( p , 0,5,0,5,0,5);
//  b1.process();
//  printf("regNum:%d\n", b1.regNum);
//  //printf("HSV match: %d\n", b1.checkpoint(38*b1.width + 463, b1.image));
//  printf("centX: %d, centY: %d\n",b1.regList[0].centerX, b1.regList[0].centerY);
//  printf("centX: %d, centY: %d\n",b1.regList[1].centerX, b1.regList[1].centerY);
//  printf("centX: %d, centY: %d\n",b1.regList[2].centerX, b1.regList[2].centerY);
//  return 0;

// }


region::region(){
    num = 0;
    centerX = 0;
    centerY = 0;
    area = 0;
    maxX = 0;
    maxY = 0;
    minX = 0;
    minY = 0;

}

region::region(int x, int y, int regNum){
    num = regNum;
    centerX = x;
    centerY = y;
    area = 1;
    maxX = x;
    maxY = y;
    minX = x;
    minY = y;

}

void region::compY(int y){
    if(y > maxY){
        maxY = y;
    }
    else if(y < minY){
        minY = y;
    }

}

void region::compX(int x){
    if(x > maxX){
        maxX = x;
    }
    else if(x < minX){
        minX = x;
    }

}

void region::setCenter(){
    int Xdiff = maxX + minX;
    int Ydiff = maxY + minY;

    centerX = Xdiff/2;
    centerY = Ydiff/2;

}