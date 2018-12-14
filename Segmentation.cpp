#include <iostream>
#include "CImg.h"
#include "Segmentation.h"
#include <cmath>
#include <String>
using namespace std;
using namespace cimg_library;

#define COMPRESSIONRATIO 0.2

Segmentation::Segmentation (string filename) {
	CImg<unsigned char> temp("3.bmp");
	img = temp;
	// 缩小图片
	smallHeight = img.height() * COMPRESSIONRATIO;
	smallWidth = img.width() * COMPRESSIONRATIO;
	img.resize(smallWidth, smallHeight);
	// img.display();
	CImg<unsigned char> grayImg = getGrayImage();
	// grayImg.display();
	int threshold = otsu(grayImg);
	testSegmentation(grayImg, threshold);
}

Segmentation::~Segmentation () {
	
}

CImg<unsigned char> Segmentation::getGrayImage() {
	CImg<unsigned char> grayscaled(img.width(), img.height(), img.depth());
	grayscaled = img;
	cimg_forXY(grayscaled, i, j) {
		int r = grayscaled(i, j, 0, 0);
		int g = grayscaled(i, j, 0, 1);
		int b = grayscaled(i, j, 0, 2);
		int gray = (int)(r * 0.2126 + g * 0.7152 + b * 0.0722);
		grayscaled(i, j, 0, 0) = gray;
		grayscaled(i, j, 0, 1) = gray;
		grayscaled(i, j, 0, 2) = gray;
	}
	return grayscaled;
}

int Segmentation::otsu(const CImg<unsigned char>& grayImg) {
	int pixelCount[256];
	double pixelRatio[256];

	// initialize
	for (int i = 0; i < 256; i++) {
		pixelCount[i] = 0;
	}
	cimg_forXY(grayImg, x, y) {
		pixelCount[grayImg(x,y,0,0)]++;
	}

	// 获得256个灰度级的比例并求出最大值
	float maxPixelRatio = 0.0;
	int maxPixelValue = 0;
	for(int i = 0; i < 256; i++) {
		pixelRatio[i] = (double)pixelCount[i]/(smallWidth*smallHeight);
		if(pixelRatio[i] > maxPixelRatio) {
			maxPixelRatio = pixelRatio[i];
			maxPixelValue = i;
		}
	}

	float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
	int threshold = 0;
    for (int i = 0; i < 256; i++)    
    {
        w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
        for (int j = 0; j < 256; j++)
        {
            if (j <= i)   //背景部分  
            {
                w0 += pixelRatio[j];
                u0tmp += j * pixelRatio[j];
            }
            else        //前景部分  
            {
                w1 += pixelRatio[j];
                u1tmp += j * pixelRatio[j];
            }
        }
        u0 = u0tmp / w0;
        u1 = u1tmp / w1;
        u = u0tmp + u1tmp;
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);
        if (deltaTmp > deltaMax)
        {
            deltaMax = deltaTmp;
            threshold = i;
        }
    }
    cout << "the threshold calculated by otsu is " << threshold << endl;
	return threshold;
}

void Segmentation::testSegmentation(const CImg<unsigned char>& grayImg, const int& threshold) {
	// 如果八邻域中同时存在黑色和白色，则为边缘
	int moreThanThreshold = 0;
	int lessThenThreshold = 0;
	CImg<unsigned char> temp = grayImg;
	for(int i = 1; i < smallWidth - 1; i++ ) {
		for (int j = 1; j < smallHeight - 1; j++) {
			// 上、下、左、右
			moreThanThreshold = 0;
			lessThenThreshold = 0;
			if(grayImg(i,j-1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i,j+1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i-1,j,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i+1,j,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i-1,j-1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i-1,j+1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i+1,j-1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(grayImg(i+1,j+1,0,0) <= threshold) {
				lessThenThreshold++;
			} else {
				moreThanThreshold++;
			}
			if(lessThenThreshold < 8 && moreThanThreshold < 8) {
				// 描红
				temp(i, j, 0, 0) = 255;
				temp(i, j, 0, 1) = 0;
				temp(i, j, 0, 2) = 0;
			}
			
		}
	}
	// cimg_forXY(grayImg, i, j) {
	// 	if(grayImg(i,j,0,0) <= threshold) {
	// 		grayImg(i, j, 0, 0) = 0;
	// 		grayImg(i, j, 0, 1) = 0;
	// 		grayImg(i, j, 0, 2) = 0;
	// 	} else {
			
	// 	}
	// }
	temp.display();
}