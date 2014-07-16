#include "face.h"

FaceDetector::FaceDetector():Detector(FACE)
{
	faceClassifier = CascadeClassifier("haarcascade_frontalface_default.xml");
}

void FaceDetector::detect(const Mat &frame)
{
	cvtColor(frame, gray, COLOR_BGR2GRAY);
	faceClassifier.detectMultiScale(frame, detection);
}