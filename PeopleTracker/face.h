#ifndef _FACE_H
#define _FACE_H
#include "opencv2/opencv.hpp"
#include "detector.h"

class FaceDetector : public Detector
{
public:
	FaceDetector();
	virtual void detect(const Mat& frame);
private:
	CascadeClassifier faceClassifier;
	Mat gray;
};
#endif