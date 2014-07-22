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
		circle(tempMap, center, HEAT_RADIUS, Scalar(20), -1);
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

void Heatmap::drawHeatImg(Mat &frame)
{
	Mat heatImg = getHeatImg();
	Mat alphaImg = Mat::zeros(heatImg.rows, heatImg.cols, CV_8UC4);
	for (int i = 0; i < heatImg.rows; i++) {
		for (int j = 0; j < heatImg.cols; j++) {
			cv::Vec3b color = heatImg.at<cv::Vec3b>(i, j);
			if (color[0] == 128 && color[1] == 0 && color[2] == 0) {
				alphaImg.at<cv::Vec4b>(i, j) = Scalar::all(0);
			} else {
				Vec4b &pixel = alphaImg.at<cv::Vec4b>(i, j);
				pixel = Scalar(color[0], color[1], color[2], 100);
			}
		}
	}

	Mat result;
	overlayImg(frame, alphaImg, result, Point2i(0, 0));

	frame = result;
}

void Heatmap::overlayImg(const cv::Mat &background, const cv::Mat &foreground,
	cv::Mat &output, cv::Point2i &location)
{
	background.copyTo(output);
	// start at the row indicated by location, or at row 0 if location.y is negative.
	for (int y = std::max(location.y, 0); y < background.rows; ++y) {
		int fY = y - location.y; // because of the translation

		// we are done of we have processed all rows of the foreground image.
		if (fY >= foreground.rows)
			break;

		// start at the column indicated by location, 

		// or at column 0 if location.x is negative.
		for (int x = std::max(location.x, 0); x < background.cols; ++x) {
			int fX = x - location.x; // because of the translation.

			// we are done with this row if the column is outside of the foreground image.
			if (fX >= foreground.cols)
				break;

			// determine the opacity of the foregrond pixel, using its fourth (alpha) channel.
			double opacity =
				((double)foreground.data[fY * foreground.step + fX * foreground.channels() + 3])

				/ 255.;


			// and now combine the background and foreground pixel, using the opacity, 

			// but only if opacity > 0.
			for (int c = 0; opacity > 0 && c < output.channels(); ++c) {
				unsigned char foregroundPx =
					foreground.data[fY * foreground.step + fX * foreground.channels() + c];
				unsigned char backgroundPx =
					background.data[y * background.step + x * background.channels() + c];
				output.data[y*output.step + output.channels()*x + c] =
					backgroundPx * (1. - opacity) + foregroundPx * opacity;
			}
		}
	}
}
