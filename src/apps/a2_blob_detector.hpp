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
	blob_detector(image_u32_t *im, int H1, int H2, int S1, int S2, int V1, int V2);

	void bucketfill(int curX, int curY, image_u32_t * im, deque<int> *nextcoord, region * reg);
	bool checkpoint(int t, image_u32_t * im);
	void HSV(uint32_t val, int * H, double * S, double *V);
	void process();


};
#endif