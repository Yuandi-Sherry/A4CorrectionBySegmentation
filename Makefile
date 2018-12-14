run: main.cpp Segmentation.cpp
	g++ -o 1.exe Segmentation.cpp main.cpp HoughTransform.cpp Matrix.cpp -lgdi32
	./1.exe