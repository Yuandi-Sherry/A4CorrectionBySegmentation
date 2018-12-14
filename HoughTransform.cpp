#include "HoughTransform.h"
#include "CImg.h"
#include "canny.h"
#include <iostream>
#include <cmath>
#include <String>

#define M_PI 3.14159265358979323846
using namespace cimg_library;
using namespace std;
HoughTransform::HoughTransform(CImg<unsigned char> inputImg, CImg<unsigned char> origin, int id, int thres) {
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
	cout << deltaTheta << endl;
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
	generateLines();
	pointsOnLine();
	drawPoints();
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
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			//cout << img(i, j) << endl;
			if (img(i, j) == 255) {
				for (int theta = 0; theta < 360; theta++) {
					int ro = abs(round(i*cosTheta[theta] + j * sinTheta[theta]));
					accumulation(theta, ro) = accumulation(theta, ro) + 1;

				}
			}
		}
	}
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
	/*for (int i = 0; i < buffer.size(); ) {
		double k = 0, b = 0;
		int x1 = 0, x2 = 0, y1 = 0, y2 = 0;
		if (sinTheta[buffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[buffer[i].first] / sinTheta[buffer[i].first];
			b = buffer[i].second / sinTheta[buffer[i].first];
			linesParams.push_back(make_pair(k, b));
		}
		else {
			b = buffer[i].second;
		}
		bool first = true;
		for (int j = 0; j < width; j++) {
			if(k*j + b >= 0 && k*j + b < height)
			if (img(j, k*j + b) == 255) {
				if (first) {
					first = false;
					x1 = j;
					y1 = k * j + b;
				}
				else {
					x2 = j;
					y2 = k * j + b;
				}
			}
		}
		int dist = sqrt(pow(x1 - x2, 2) + pow(y1 - y1, 2));
		if (dist < maxLength) {
			count++;
			buffer.erase(buffer.begin() + i);
		}
		else {
			i++;
		}
	}*/

	for (int i = 0; i < buffer.size(); ) {
		boolean eraseI = false;
		for (int j = i + 1; j < buffer.size(); ) {
			// 在同一单调区间比较
			if (((buffer[i].first > 0 && buffer[i].first < 180 && buffer[j].first > 0 && buffer[j].first < 180 && abs(buffer[i].first - buffer[j].first) < deltaTheta)
				|| (buffer[i].first > 180 && buffer[i].first < 360 && buffer[j].first > 180 && buffer[j].first < 360 && abs(buffer[i].first - buffer[j].first) < deltaTheta)
				|| (buffer[i].first > 180 && buffer[i].first < 360 && buffer[j].first > 0 && buffer[j].first < 180 && abs(buffer[i].first - buffer[j].first - 180) < deltaTheta)
				|| (buffer[i].first > 0 && buffer[i].first < 180 && buffer[j].first > 180 && buffer[j].first < 360 && abs(buffer[j].first - buffer[i].first - 180) < deltaTheta))
				&& (abs(buffer[i].second - buffer[j].second) < deltaRho || abs(buffer[i].second + buffer[j].second) < deltaRho)) {
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

void HoughTransform::generateLines() {
	
	//draw
	for (int i = 0; i < buffer.size(); i++) {
		double k = 0, b = 0;
		if (sinTheta[buffer[i].first] != 0) {
			k = (-1) * (double)cosTheta[buffer[i].first] / sinTheta[buffer[i].first];
			b = buffer[i].second / sinTheta[buffer[i].first];
			linesParams.push_back(make_pair(k, b));
		}
		else {
			b = buffer[i].second;
		}
		cout << "y = " << k << " * x + " << b << endl;
	}
//	cout << "size  " << linesParams.size() << endl;

	const double blue[] = { 0, 0, 255 };
	const double green[] = { 0, 255, 0 };
	for (int i = 0; i < linesParams.size(); i++) {
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
			edge.draw_line(x0, 0, x1, height, green);
        }
	}

	string t = to_string(id) + "paperLines_origin.bmp";
	const char * temp = t.c_str();
	result.save(temp);

	string t1 = to_string(id) + "-I2.bmp";
	const char * temp1 = t1.c_str();
	edge.save(temp1);
}

void HoughTransform::pointsOnLine() {
	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			if (edge(i, j, 0, 2) == 0
				&& edge(i, j, 0, 1) == 255
				&& edge(i, j, 0, 0) == 0) {
				if (img(i, j) == 255) {
					edge(i, j, 0, 0) = 255;
					edge(i, j, 0, 1) = 0;
					edge(i, j, 0, 2) = 0;
				}	
			}
		}
	}

	string t1 = to_string(id) + "-I3.bmp";
	const char * temp1 = t1.c_str();
	edge.save(temp1);
	
}
void HoughTransform::drawPoints() {
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
	const double pointColor[] = { 255, 0, 0 };
	//cout << "draw points" << " " << points.size() << endl;
	for (int i = 0; i < points.size(); i++) {
		result.draw_circle(points[i].first, points[i].second, 5, pointColor);
		edge.draw_circle(points[i].first, points[i].second, 5, pointColor);
	}
	//result.display(); 
	string t = to_string(id) + "paperPoint_origin.bmp";
	const char * temp = t.c_str();
	result.save(temp);

	string t1 = to_string(id) + "-I4.bmp";
	const char * temp1 = t1.c_str();
	edge.save(temp1);
}