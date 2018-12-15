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
	void sortPoints(std::vector<pair<int,int>> & points);
	Matrix getHMatrix(const vector<pair<int,int>> & points) ;
	CImg<unsigned char> warpingA4(Matrix & H);

private:
	// 用于缩小处理的图片
	CImg<unsigned char> img;
	// 原图
	CImg<unsigned char> origin;
	int smallHeight;
	int smallWidth;

	int width;
	int height;
};
#endif