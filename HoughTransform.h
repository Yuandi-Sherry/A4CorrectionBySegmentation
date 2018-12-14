#ifndef HOUGH_TRANSFORM
#define HOUGH_TRANSFORM
#include "CImg.h"
#include <vector>
#include <iostream>
using namespace std;
using namespace cimg_library;

class HoughTransform {
private:
	CImg<unsigned char> img;
	CImg<unsigned char> accumulation;
	CImg<unsigned char> result;
	CImg<unsigned char> edge;
	int id;
	int width;
	int height;
	int dia; // 原图片对角线长度
	int dis;
	int maxLength;
	double deltaTheta;
	double deltaRho;
	double* sinTheta;
	double* cosTheta;
	vector<pair<int, int>> buffer;
	vector<pair<double, double>> linesParams;
	vector<pair<int, int>> points;
	int threshold;

public:
	HoughTransform(CImg<unsigned char> img, CImg<unsigned char> origin, int id, int thres);
	void run();
	void initTriangle();
	void findLocalMaximums(int);
	void fillAccumulation();
	void generateLines();
	void drawPoints();
	void filter();
	void pointsOnLine();
};

#endif // !HOUGH_TRANSFORM
