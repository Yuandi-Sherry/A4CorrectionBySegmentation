#include "HoughTransform.h"
#include "CImg.h"
#include <iostream>
#include <cmath>
#include <String>

#define M_PI 3.14159265358979323846
using namespace cimg_library;
using namespace std;
HoughTransform::HoughTransform(const CImg<unsigned char> & inputImg, const CImg<unsigned char> & origin, int id, int thres) {
	this->id = id;
	img = inputImg;
	result = origin;
	
	width = img.width();
	height = img.height();
	edge = CImg<unsigned char>(width, height,1, 3);
	cimg_forXY(edge, i, j) {
		if (img(i, j) == 255) {
			edge(i, j, 0, 0) = 255;
			edge(i, j, 0, 1) = 255;
			edge(i, j, 0, 2) = 255;
		}
		else {
			edge(i, j, 0, 0) = 0;
			edge(i, j, 0, 1) = 0;
			edge(i, j, 0, 2) = 0;
		}
	}
	double temp = width * width + height * height;
	dia = ceil(sqrt(temp));
	deltaTheta = 15;
	deltaRho = dia / 25;
	dis = 80;
	threshold = thres;
	maxLength = dia / 10;
	accumulation = CImg<unsigned char>(360, dia);

	string t = to_string(id) + "edge_origin.bmp";
	const char * tempName = t.c_str();
	img.save(tempName);
	initTriangle();
	fillAccumulation();
	findLocalMaximums(threshold);
	filter();
	generateLines(dia);
	// getPointsOnLine();
	// getPoints();
}


void HoughTransform::initTriangle() {

	sinTheta = new double[360];
	cosTheta = new double[360];
	for (int i = 0; i < 360; i++) {
		sinTheta[i] = sin(i*M_PI / 180);
		cosTheta[i] = cos(i*M_PI / 180);
	}

	for (int i = 0; i < accumulation.width(); i++) {
		for (int j = 0; j < accumulation.height(); j++) {
			accumulation(i, j) = 0;
		}
	}
}

void HoughTransform::fillAccumulation() {
	cout << "fill accumulat`ion" << endl;
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//cout << img(i, j) << endl;
			if (img(i, j,0,1) == 255 ) {
				for (int theta = 0; theta < 360; theta++) {
					int ro = round(i*cosTheta[theta] + j * sinTheta[theta]);
					int tempTheta = theta;
					if(ro < 0) {
						tempTheta = theta - 180 < 0 ? theta + 180: theta  - 180;
					}
					accumulation(tempTheta, abs(ro)) = accumulation(tempTheta, abs(ro)) + 1;
				}
			}
		}
	}
	cout << "fill accumulation" << endl;
}

void HoughTransform::findLocalMaximums(int threshold) {
	for (int r = 0; r < dia; r++) {
		for (int theta = 0; theta < 360; theta++) {
			bool push = true;
			if (accumulation(theta, r) > threshold) {
				for (int i = 0; i < buffer.size(); i++) {
					if (sqrt(pow(buffer[i].first - theta, 2) + pow(buffer[i].second - r, 2)) < dis) {
						push = false;
						if (accumulation(buffer[i].first, buffer[i].second) <
							accumulation(theta, r)) {							
							buffer[i] = make_pair(theta, r);
						} 
					}
				}
				if (push) {
					buffer.push_back(make_pair(theta, r));
				}
			}
		}
	}
}

void HoughTransform::filter() {
	int count = 0;
	for (int i = 0; i < buffer.size(); ) {
		boolean eraseI = false;
		for (int j = i + 1; j < buffer.size(); ) {
			// 比较theta相近的
			if (abs(buffer[i].first - buffer[j].first) < deltaTheta || 
				abs(abs(buffer[i].first - buffer[j].first) - 180) < deltaTheta || 
				abs(abs(buffer[i].first - buffer[j].first) - 360) < deltaTheta) {
					// 判断交点在图片内
					double k1, k2, b1, b2;
					if (sinTheta[buffer[i].first] != 0) {
						k1 = (-1) * (double)cosTheta[buffer[i].first] / sinTheta[buffer[i].first];
						b1 = buffer[i].second / sinTheta[buffer[i].first];
					}
					else {
						k1 = 100;
						b1 = buffer[i].second;
					}
					if (sinTheta[buffer[j].first] != 0) {
						k2 = (-1) * (double)cosTheta[buffer[j].first] / sinTheta[buffer[j].first];
						b2 = buffer[j].second / sinTheta[buffer[j].first];
					}
					else {
						k2 = 100;
						b2 = buffer[j].second;
					}
					if(this->id == 2) {
						cout << "*********delta similar*********" << deltaTheta<< endl;
						cout << "theta = " << buffer[i].first << " theta = " << buffer[j].first << endl;
						cout << "y = " << k1 << " * x + " << b1 << endl;
						cout << "y = " << k2 << " * x + " << b2 << endl;
						cout << accumulation(buffer[i].first, buffer[i].second,0,0) << endl;
						cout << accumulation(buffer[j].first, buffer[j].second,0,0) << endl;
						
					}
					
					// 计算交点
					const int x = (double)(-b2+b1)/(-k1+k2);
					const int y = (double)(-k2*(-b1) + k1*(-b2))/(-k1+k2);

					cout << "x = " <<  x << " y = " << y << endl;
					if(x>=0 && y >= 0 && x < width && y < height){
						cout << "cross point is in the image" << endl;
						count++;
						if (accumulation(buffer[i].first, buffer[i].second) <
							accumulation(buffer[j].first, buffer[j].second)) {
							buffer.erase(buffer.begin() + i);
							eraseI = true;
						}
						else if (accumulation(buffer[j].first, buffer[j].second) <=
							accumulation(buffer[i].first, buffer[i].second)) {
							buffer.erase(buffer.begin() + j);
							continue;
						}
					}
				}
			j++;
		}
		if (!eraseI) {
			i++;
		}
		else {
			i--;
		}
	}
}

void HoughTransform::generateLines(const int & dia) {
	//draw
	const double red[] = { 255, 255, 0};
	for (int i = 0; i < buffer.size(); i++) {
		double k = 0, b = 0;
		// if (sinTheta[buffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[buffer[i].first] / sinTheta[buffer[i].first];
			b = buffer[i].second / sinTheta[buffer[i].first];
			// 点到直线的距离要小于对角线长度
			if(abs(-b/sqrt(pow(k,2)+1)) < dia) {
				cout << "y = " << k << " * x + " << b << endl;
				linesParams.push_back(make_pair(k, b));
			}
		// }
		// else {
		// 	k = 100;
		// 	b = buffer[i].second;
		// 	cout << "y = " << k << " * x + " << b << endl;
		// 	linesParams.push_back(make_pair(k, b));
		// }
	}

	const double blue[] = { 0, 0, 255 };
	const double green[] = { 0, 255, 0 };
	for (int i = 0; i < linesParams.size(); i++) {
		// if(linesParams[i].first != 0) {
			const int x0 = (double)(0 - linesParams[i].second) / linesParams[i].first;
	        const int x1 = (double)(height - linesParams[i].second) / linesParams[i].first;
	        const int y0 = 0*linesParams[i].first + linesParams[i].second;
	        const int y1 = width*linesParams[i].first + linesParams[i].second;

	        if (abs(linesParams[i].first) > 1) {
	            result.draw_line(x0, 0, x1, height, blue);
				edge.draw_line(x0, 0, x1, height, green);
	        }
	        else {
	            result.draw_line(0, y0, width, y1, blue);
				edge.draw_line(0, y0, width, y1, green);
	        }
		// } else {
		// 	edge.draw_line(0, (int)linesParams[i].second, width-1, (int)linesParams[i].second, red);
		// 	result.draw_line(0, (int)linesParams[i].second, width-1, (int)linesParams[i].second, red);
		// }
		
	}

	string t = to_string(id) + "paperLines_origin.bmp";
	const char * temp = t.c_str();
	result.save(temp);

	string t1 = to_string(id) + "-I2.bmp";
	const char * temp1 = t1.c_str();
	edge.save(temp1);
}

// void HoughTransform::getPointsOnLine() {
// 	for (int i = 0; i < width; i++) {
// 		for (int j = 0; j < height; j++) {
// 			if (edge(i, j, 0, 2) == 0
// 				&& edge(i, j, 0, 1) == 255
// 				&& edge(i, j, 0, 0) == 0) {
// 				if (img(i, j) == 255) {
// 					edge(i, j, 0, 0) = 255;
// 					edge(i, j, 0, 1) = 0;
// 					edge(i, j, 0, 2) = 0;
// 				}	
// 			}
// 		}
// 	}

// 	string t1 = to_string(id) + "-I3.bmp";
// 	const char * temp1 = t1.c_str();
// 	edge.save(temp1);
	
// }
vector<pair<int, int>> HoughTransform::getPoints() {
	for (int i = 0; i < linesParams.size(); i++) {
		for (int j = i + 1; j < linesParams.size(); j++) {
			if (linesParams[i].first != linesParams[j].first) {
				double x = (-1) * (linesParams[i].second - linesParams[j].second) / (linesParams[i].first - linesParams[j].first);
				int y = linesParams[i].first * x + linesParams[i].second;
				if (x >= 0 && x < width && y >= 0 && y < height) {
					points.push_back(make_pair((int)x + 3, y + 3));
				}
			}
		}
	}
	return points;
	// const double pointColor[] = { 255, 0, 0 };
	// //cout << "draw points" << " " << points.size() << endl;
	// for (int i = 0; i < points.size(); i++) {
	// 	result.draw_circle(points[i].first, points[i].second, 5, pointColor);
	// 	edge.draw_circle(points[i].first, points[i].second, 5, pointColor);
	// }
	// //result.display(); 
	// string t = to_string(id) + "paperPoint_origin.bmp";
	// const char * temp = t.c_str();
	// result.save(temp);

	// string t1 = to_string(id) + "-I4.bmp";
	// const char * temp1 = t1.c_str();
	// edge.save(temp1);
}