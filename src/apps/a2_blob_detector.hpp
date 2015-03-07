#ifndef __a2_blob_detector_H__
#define __a2_blob_detector_H__

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

	private:
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
	int Xdiff = maxX - minX;
	int Ydiff = maxY - minY;

	centerX = Xdiff/2;
	centerY = Ydiff/2;

}
#endif