#include <iostream>
#include "CImg.h"
#include "Segmentation.h"
#include "HoughTransform.h"
#include "Matrix.h"
#include <cmath>
#include <String>
using namespace std;
using namespace cimg_library;

#define COMPRESSIONRATIO 0.2
#define HOUGHTHRESHOLD 100

bool comp1(const pair<int,int> &a, const pair<int,int> &b){
	if(a.first < b.first) {
		return 1;
	} else {
		return 0;
	}
}

bool comp2(const pair<int,int> &a, const pair<int,int> &b){
	if(a.second < b.second) {
		return 1;
	} else {
		return 0;
	}
}
Segmentation::Segmentation (string filename) {
	CImg<unsigned char> temp(filename.c_str());
	img = temp;
	// 缩小图片
	smallHeight = img.height() * COMPRESSIONRATIO;
	smallWidth = img.width() * COMPRESSIONRATIO;
	img.resize(smallWidth, smallHeight);
	// img.display();
	CImg<unsigned char> grayImg = getGrayImage();
	
	// 获得图像分割阈值
	int threshold = otsu(grayImg);
	// 分割图像并进行边缘提取
	CImg<unsigned char> edgeImg = testSegmentation(grayImg, threshold);
	edgeImg.save( ("Edge" + filename).c_str());
	// Hough变换获得直线的四个角
	HoughTransform houghTransform = HoughTransform(edgeImg, img, filename[0] - '0', 150);
	std::vector<pair<int,int>> points = houghTransform.getPoints();
	// 对点按照左上左下，右上右下排序
	sort(points.begin(),points.end(),comp1);
	sort(points.begin(),points.end()-2,comp2);
	sort(points.begin()+2,points.end(),comp2);
	// 从这一步开始改用原图，睡醒起来改
	for(int i = 0; i < points.size(); i++) {
		points[i].first /= COMPRESSIONRATIO;
		points[i].second /=COMPRESSIONRATIO;
	}
	Matrix H(getHMatrix(points));
	cout << H << endl;
	warpingA4(H);
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

CImg<unsigned char> Segmentation::testSegmentation(CImg<unsigned char>& grayImg, const int& threshold) {
	// 如果八邻域中同时存在黑色和白色，则为边缘
	int moreThanThreshold = 0;
	int lessThenThreshold = 0;
	CImg<unsigned char> temp = grayImg;
	// 将分割的图像用黑色和白色填充
	cimg_forXY(grayImg, i, j) {
		temp(i, j, 0, 0) = 0;
		temp(i, j, 0, 1) = 0;
		temp(i, j, 0, 2) = 0;
		if(grayImg(i,j,0,0) <= threshold) {
			grayImg(i, j, 0, 0) = 0;
			grayImg(i, j, 0, 1) = 0;
			grayImg(i, j, 0, 2) = 0;
			
		} else {
			grayImg(i, j, 0, 0) = 255;
			grayImg(i, j, 0, 1) = 255;
			grayImg(i, j, 0, 2) = 255;
		}
	}
	// temp.save("afterColor.bmp");
	// 补全阴影
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
			if(moreThanThreshold > 4) {
				// 若八邻域一半以上均为白色，则说明该点为白纸
				grayImg(i, j, 0, 0) = 255;
				grayImg(i, j, 0, 1) = 255;
				grayImg(i, j, 0, 2) = 255;
			} else if(lessThenThreshold > 4) {
				grayImg(i, j, 0, 0) = 0;
				grayImg(i, j, 0, 1) = 0;
				grayImg(i, j, 0, 2) = 0;
			}
			
			
		}
	}
	// grayImg.save("afterFill.bmp");
	// 对边缘描红
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
				temp(i, j, 0, 1) = 255;
				temp(i, j, 0, 2) = 255;
			}
			
		}
	}
	// temp.save("edge.bmp");
	return temp;
}

Matrix Segmentation::getHMatrix(const vector<pair<int,int>> & points) {
	// 在边缘检测中的四个坐标

	double data[64];
	// 映射后坐标
	double vecB[8] = {0,0,0,smallHeight, smallWidth,0,smallWidth,smallHeight};
	for (int i = 0; i < 64; i += 16) {
		data[i] = data[i + 11] = points[i/16].first;
		data[i + 1] = data[i + 12] = points[i/16].second;
		data[i + 2] = data[i + 13] = 1;
		data[i + 3] = data[i + 4] = data[i + 5] = data[i + 8] = data[i + 9] = data[i + 10] = 0;
		data[i + 6] = vecB[i/16*2] * points[i/16].first;
		data[i + 7] = vecB[i/16*2] * points[i/16].second;
		data[i + 14] = -vecB[i/16*2+1] * points[i/16].first;
		data[i + 15] = -vecB[i/16*2+1] * points[i/16].second;
	}
	Matrix A(data, 8, 8);
	Matrix b(vecB, 8, 1);
	Matrix result = A.Inverse()*b;

	double Hdata[9];
	for (int i = 0; i < 8; i++) {
		Hdata[i] = result.item[i];
	}
	Hdata[8] = 1;
	Matrix H(Hdata, 3, 3);
	return H;
}

void Segmentation::warpingA4(Matrix & H) {
	CImg<unsigned char> result = CImg<unsigned char>(smallWidth, smallHeight, 1, 3);
	cimg_forXY(result, x, y) {
		double xy[3] = {x,y,1};
		const Matrix xyM(xy,3,1);
		Matrix xyPrime = H.Inverse()*xyM;
		if(xyPrime.item[0]/xyPrime.item[2] >=0 && xyPrime.item[0]/xyPrime.item[2] < smallWidth && xyPrime.item[1]/xyPrime.item[2] >=0 && xyPrime.item[1]/xyPrime.item[2]<smallHeight){
			result(x,y,0,0) = img(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,0);
			result(x,y,0,1) = img(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,1);
			result(x,y,0,2) = img(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,2);

		}		
	}
	result.display();
}