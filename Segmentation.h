#ifndef _SEGMENTATION_H_
#define _SEGMENTATION_H_
#include <iostream>
#include "CImg.h"
#include "HoughTransform.h"
#include "Segmentation.h"
#include "Matrix.h"
#include <cmath>
#include <String>
using namespace std;
using namespace cimg_library;
class Segmentation
{
public:
	Segmentation(string filename);
	~Segmentation();
	/**
	 * get the threshold of image segmentation
	 */
	int otsu();
	CImg<unsigned char> getGrayImage();
	int otsu(const CImg<unsigned char>& grayImg) ;
	CImg<unsigned char> testSegmentation(CImg<unsigned char>& grayImg, const int& threshold);
	Matrix getHMatrix(const vector<pair<int,int>> & points) ;
	void warpingA4(Matrix & H);
private:
	CImg<unsigned char> img;
	int smallHeight;
	int smallWidth;
};
#endif