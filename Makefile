run: main.cpp Segmentation.cpp
	g++ -o 1.exe Segmentation.cpp main.cpp -lgdi32
	./1.exe