#include "CImg.h"
#include "Segmentation.h"
#include <iostream>
#include <String>
using namespace std;
using namespace cimg_library;

int main() {

	for(int i = 1; i <= 6; i++) {
		string filename = to_string(i) + ".bmp";
		Segmentation segmentation(filename);
	}
	
}