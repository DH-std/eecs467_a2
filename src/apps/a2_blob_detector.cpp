#include <math.h>
#include <stdlib.h>
#include "a2_blob_detector.hpp"
#include <lcm/lcm-cpp.hpp>
#include <deque>
#include <vector>
#include <imagesource/image_u32.h>

using namespace std;

#define minArea 10 //min area to be considered a region


int Hmin, Hmax, Smin, Smax, Vmin, Vmax;
vector<int> vis;
vector<region> regList;
int regNum;
int width, height, stride;

void HSV(int val, int * H, double * S, double *V){

	//convert to rgba first

	double r, g, b, a;
	a = val / (1<<24);
	val = val % (1<<24);
	b = val / (1<<16) ;
	val = val % 16;
	g = val / (1 << 8);
	val = val % 8;
	r = val;

	r = r/255;
	g = g/255;
	b = b/255;

	double cmax, cmin, delta;
	int col = 0;

	if(r >=  b &&  r >= g){
		cmax = r;
		col = 1;
	}
	else if( g >=r  && g >= a){
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
	else if( g <=r  && g <= a){
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
		*H = (g-b)/delta;
		*H = *H%6;
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
	}
	*V = cmax;
}	

bool checkpoint(int t, image_u32_t * im){
	double  S, V;
	int H;
	HSV(t, &H, &S, &V);
	int Hval = H;
	int Sval = S;
	int Vval = V;

	if((Hval < Hmax) && (Hval > Hmin) ){
		if((Sval < Smax) && (Sval > Smin)){
			if((Vval < Vmax) && (Vval > Vmin)){
				return true;
			}
		}
	}
	return false;
}

void bucketfill(int curX, int curY, image_u32_t * im,
					 deque<int> *nextcoord, region * reg){


	//search surrounding neighbors
	for(int i = 0; i < 8; i++){
		int a,b;
		switch(i){
			case 0:
				a = -1;
				b = 1;
				break;
			case 1:
				a = 0;
				b = 1;
				break;
			case 2:
				a = 1;
				b = 1;
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
				b = -1;
				break;
			case 6:
				a = 0;
				b = -1;
				break;
			case 7:
				a = 1;
				b = -1;
				break;
		}


		if( vis[(curY + b)*stride + (curX+a)] == false && checkpoint((curY + b)*stride + (curX+a), im)){
			nextcoord->push_back((curY + b)*stride + (curX+a));
			vis[(curY + b)*stride + (curX+a)] = true;
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

		bucketfill(curX, curY, im, nextcoord, reg);
	}

}

int main(){

	//initialize HSV values from task2

	Hmin = 55;
	Hmax = 75;
	Smin = 30;
	Smax = 35;
	Vmin = 60;
	Vmax = 65;

	regNum = 1;

	//load pnm image into image_u32_t
	char path[100] = "path_name"; // need to insert path name here
	image_u32_t* image = image_u32_create_from_pnm(path);

	width = image->width;
	height = image->height;
	stride = image->stride;

	vis.assign(width*height, 0);

	//looping to find start positions
	for(int y=0; y < image->height; y++){
		for(int x=0; x < image->width; x++){

			if(vis[y*stride+x] == false && checkpoint(y*stride+x, image)){

				deque<int> nextcoord;
				region temp_reg(x, y, regNum);

				vis[y*stride+x] = true;
				bucketfill(x, y, image , &nextcoord, &temp_reg );

				if(temp_reg.area >= minArea){
					temp_reg.setCenter();
					regList.push_back(temp_reg);
					regNum++;
				}

			}
			else{
				vis[y*stride+x] = true;

			}

		}
	}


return 0;	
}