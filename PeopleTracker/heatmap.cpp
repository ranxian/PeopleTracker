#include "heatmap.h"
#include "parameter.h"

Heatmap::Heatmap(int height_, int width_):width(width_), height(height_)
{
	densityMap = Mat::zeros(height, width, CV_8U);
	tempMap = Mat::zeros(height, width, CV_8U);
}

void Heatmap::feed(vector<Result2D> &results)
{
	vector<Result2D>::iterator it;

	for (it = results.begin(); it != results.end(); it++) {
		Point center((*it).xc, (*it).yc + (*it).h/2);
		circle(tempMap, center, HEAT_RADIUS, Scalar(5), -1);
		GaussianBlur(tempMap, tempMap, Size(99, 99), 0);
		densityMap += tempMap;
		tempMap.setTo(0);
	}
}

Mat Heatmap::getHeatImg()
{
	Mat heatImg = densityMap.clone();
	applyColorMap(heatImg, heatImg, COLORMAP_JET);
	return heatImg;
}