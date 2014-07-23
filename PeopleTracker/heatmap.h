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
	void drawHeatImg(Mat &frame);
	void feed(vector<Result2D> &results);
	int RADIUS;
private:
	Mat densityMap;
	Mat tempMap;
	int width;
	int height;
	void overlayImg(const cv::Mat &bg, const cv::Mat &fg, cv::Mat &result, cv::Point2i &location);
};

#endif