#ifndef _FACE_H
#define _FACE_H
#include "opencv2/opencv.hpp"
#include "detector.h"
#include <pittpatt_sdk.h>
#include <pittpatt_license.h>

// Helper
ppr_error_type cv2pprimg(const Mat &frame, ppr_image_type *image);
bool ppr2cvimg(ppr_image_type *image, Mat &frame);
// SDK init and term
bool init_ppr_sdk();
void finalize_sdk();
extern ppr_context_type ppr_context;

class FaceDetector
{
public:
	FaceDetector();
	~FaceDetector();
	virtual void detect(const Mat& frame);
	void drawDetection(Mat &frame);
	ppr_face_list_type getDetections();
private:
	CascadeClassifier faceClassifier;
	Mat gray;

	bool detected;

	ppr_face_list_type face_list;
};
#endif