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
	// 原图
	origin = temp;
	width = origin.width();
	height = origin.height();
	// 缩小图片
	img = temp;
	smallHeight = img.height() * COMPRESSIONRATIO;
	smallWidth = img.width() * COMPRESSIONRATIO;
	img.resize(smallWidth, smallHeight);
	// 变为灰度图像
	CImg<unsigned char> grayImg = getGrayImage();
	// 获得图像分割阈值
	int threshold = otsu(grayImg);
	// 分割图像并进行边缘提取
	CImg<unsigned char> edgeImg = testSegmentation(grayImg, threshold);
	edgeImg.save( ("Edge" + filename).c_str());
	// Hough变换
	HoughTransform houghTransform = HoughTransform(edgeImg, img, filename[0] - '0', 150);
	// 获得A4纸的四个角
	std::vector<pair<int,int>> points = houghTransform.getPoints();
	// 对点按照左上左下，右上右下排序，并改回原图比例
	sortPoints(points);
	// 获得变换矩阵
	Matrix H(getHMatrix(points));
	// 获得矫正后的图片
	CImg<unsigned char> result = warpingA4(H);
	result.save( ("Result" + filename).c_str());
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

void Segmentation::sortPoints(std::vector<pair<int,int>> & points) {
	// 计算两两距离，距离最小的点 作为上边的端点和下边的端点，根据相对位置分左右
	pair<int,int> min1;
	pair<int, int> min2;
	int minDistance1 = sqrt(width*width + height*height),minDistance2 = sqrt(width*width + height*height);
	for (int i = 0; i < points.size(); i++) {
		for(int j = i+1; j < points.size(); j++) {
			// 计算距离
			double dis = sqrt(pow(points[i].first - points[j].first, 2) + pow(points[i].second - points[j].second, 2));
			if(dis < minDistance1) {
				minDistance2 = minDistance1;
				min2.first = min1.first;
				min2.second = min1.second;
				// 最小
				minDistance1 = dis;
				min1.first = i;
				min1.second = j;
			} else if (dis < minDistance2) {
				// 倒数第二小
				minDistance2 = dis;
				min2.first = i;
				min2.second = j;
			}
		}
	}
	std::vector<pair<int,int>> temp;
	temp.clear();
	pair<int,int> point1; // 左上
	pair<int,int> point2; // 左下
	pair<int,int> point3; // 右上
	pair<int,int> point4; // 右下
	// 根据在原图中的坐标判断上边和下边
	if(min(points[min1.first].second, points[min1.second].second) < min(points[min2.first].second, points[min2.second].second)) {
		// 则min1对应的两个点为上边缘
		// 左上
		point1.first = points[min1.first].first < points[min1.second].first ? points[min1.first].first : points[min1.second].first;
		point1.second = points[min1.first].first < points[min1.second].first ?  points[min1.first].second : points[min1.second].second;
		// 右上
		point3.first = points[min1.first].first < points[min1.second].first ? points[min1.second].first : points[min1.first].first;
		point3.second = points[min1.first].first < points[min1.second].first ? points[min1.second].second : points[min1.first].second;
		// 左下
		point2.first = points[min2.first].first < points[min2.second].first ? points[min2.first].first : points[min2.second].first;
		point2.second = points[min2.first].first < points[min2.second].first ? points[min2.first].second : points[min2.second].second;	
		// 右下
		point4.first = points[min2.first].first < points[min2.second].first ? points[min2.second].first : points[min2.first].first;
		point4.second = points[min2.first].first < points[min2.second].first ? points[min2.second].second : points[min2.first].second;	
	} else {
		// min2对应的两个点为上边缘
		// 左上
		point1.first = points[min2.first].first < points[min2.second].first ? points[min2.first].first : points[min2.second].first;
		point1.second = points[min2.first].first < points[min2.second].first ? points[min2.first].second : points[min2.second].second;	
		// 左下
		point2.first = points[min1.first].first < points[min1.second].first ? points[min1.first].first : points[min1.second].first;
		point2.second = points[min1.first].first < points[min1.second].first ?  points[min1.first].second : points[min1.second].second;
		// 右上
		point3.first = points[min2.first].first < points[min2.second].first ? points[min2.second].first : points[min2.first].first;
		point3.second = points[min2.first].first < points[min2.second].first ? points[min2.second].second : points[min2.first].second;	
		// 右下
		point4.first = points[min1.first].first < points[min1.second].first ? points[min1.second].first : points[min1.first].first;
		point4.second = points[min1.first].first < points[min1.second].first ? points[min1.second].second : points[min1.first].second;
	}
	temp.push_back(point1);
	temp.push_back(point2);
	temp.push_back(point3);
	temp.push_back(point4);
	points = temp;
	// 改为原图比例
	for(int i = 0; i < points.size(); i++) {
		points[i].first /= COMPRESSIONRATIO;
		points[i].second /=COMPRESSIONRATIO;
	}
}
Matrix Segmentation::getHMatrix(const vector<pair<int,int>> & points) {
	// 在边缘检测中的四个坐标
	// x,y=> 矫正前， x',y' => 校正后
	double data[64];
	// 映射后坐标
	double vecB[8] = {0,0,0,(double)height-1, (double)width-1,0,(double)width-1,(double)height-1};
	for (int i = 0; i < 64; i += 16) {
		cout << "x " << points[i/16].first << " y " << points[i/16].second << endl;
		cout << "x' " << vecB[i/16*2] << " y' " <<vecB[i/16*2+1] << endl;
		data[i] = data[i + 11] = points[i/16].first;
		data[i + 1] = data[i + 12] = points[i/16].second;
		data[i + 2] = data[i + 13] = 1;
		data[i + 3] = data[i + 4] = data[i + 5] = data[i + 8] = data[i + 9] = data[i + 10] = 0;
		data[i + 6] = -vecB[i/16*2] * points[i/16].first;
		data[i + 7] = -vecB[i/16*2] * points[i/16].second;
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

CImg<unsigned char> Segmentation::warpingA4(Matrix & H) {
	CImg<unsigned char> result = CImg<unsigned char>(width, height, 1, 3);
	cimg_forXY(result, x, y) {
		double xy[3] = {x,y,1};
		const Matrix xyM(xy,3,1);
		Matrix xyPrime = H.Inverse()*xyM;
		if(xyPrime.item[0]/xyPrime.item[2] >=0 && xyPrime.item[0]/xyPrime.item[2] < width && xyPrime.item[1]/xyPrime.item[2] >=0 && xyPrime.item[1]/xyPrime.item[2]<height){
			result(x,y,0,0) = origin(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,0);
			result(x,y,0,1) = origin(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,1);
			result(x,y,0,2) = origin(xyPrime.item[0]/xyPrime.item[2], xyPrime.item[1]/xyPrime.item[2],0,2);

		}		
	}
	return result;
}