#ifndef _HEATMAP_H
#define _HEATMAP_H
#include <opencv2/opencv.hpp>
#include "dataReader.h"
#include <opencv2/imgproc/imgproc.hpp>
// Draw heatmap according to detection result of every frame
class Heatmap
{
public:
	Heatmap(int _width, int _height);
	Heatmap(){};
	Mat getHeatImg();
	void drawHeapImg(Mat &frame);
	void feed(vector<Result2D> &results);
	int RADIUS;
private:
	Mat densityMap;
	Mat tempMap;
	int width;
	int height;
};

#endif