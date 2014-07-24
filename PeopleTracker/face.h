#ifndef _FACE_H
#define _FACE_H
#include "opencv2/opencv.hpp"
#include "detector.h"
#include <pittpatt_sdk.h>
#include <pittpatt_license.h>

// Helper
ppr_error_type cv2pprimg(const Mat &frame, ppr_image_type *image);
bool ppr2cvimg(ppr_image_type *image, Mat &frame);

class FaceDetector
{
public:
	FaceDetector();
	~FaceDetector();
	virtual void detect(const Mat& frame);
	vector<ppr_face_attributes_type> getDetection() { return detection; }
	void drawDetection(Mat &frame);
private:
	CascadeClassifier faceClassifier;
	Mat gray;
	ppr_context_type ppr_context;

	vector<ppr_face_attributes_type> detection;
};
#endif