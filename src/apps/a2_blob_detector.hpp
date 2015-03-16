#ifndef __a2_blob_detector_H__
#define __a2_blob_detector_H__
#include <string>
#include <vector>
#include <deque>
#include "common/getopt.h"
#include <imagesource/image_u32.h>

using namespace std;

class region{
	public:
	region();
	region(int x, int y, int regNum);

	int num;
	int area;
	int centerX;
	int centerY;

	void compX(int x);
	void compY(int y);
	void setCenter();


	int maxX;
	int maxY;
	int minX;
	int minY;
};


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

class blob_detector{
public:

	int Hmin, Hmax, Smin, Smax, Vmin, Vmax;
	vector<int> vis;
	vector<region> regList;
	int regNum;
	int width, height, stride;
	image_u32_t* image;
	getopt_t *gopt;
	
	blob_detector();
	blob_detector(string path_name, int H1, int H2, int S1, int S2, int V1, int V2);

	void bucketfill(int curX, int curY, image_u32_t * im, deque<int> *nextcoord, region * reg);
	bool checkpoint(int t, image_u32_t * im);
	void HSV(uint32_t val, int * H, double * S, double *V);
	void process();


};
#endif