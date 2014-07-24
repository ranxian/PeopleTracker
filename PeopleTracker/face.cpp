#include "face.h"
#include "stdio.h"

FaceDetector::FaceDetector()
{
	ppr_error_type r;

	if ((r = ppr_initialize_sdk("models", my_license_id, my_license_key)) != PPR_SUCCESS) {
		const char *msg = ppr_error_message(r);
		cout << msg << endl;
	} else {
		cout << "PittPatt SDK initialized. Configuring..." << endl;
		ppr_settings_type setting = ppr_get_default_settings();
		ppr_detection_settings_type detection_setting = setting.detection;
		setting.detection.enable = 1;
		setting.detection.use_serial_face_detection = 1;
		if ((r = ppr_initialize_context(setting, &ppr_context)) != PPR_SUCCESS) {
			cout << ppr_error_message(r) << endl;
		}
	}
}

FaceDetector::~FaceDetector()
{
	ppr_finalize_context(ppr_context);
	ppr_finalize_sdk();
}

void FaceDetector::detect(const Mat &frame)
{
	ppr_image_type image;
	ppr_error_type err;
	ppr_face_list_type face_list;
	ppr_face_attributes_type face_attr;
	ppr_face_type face;
	
	detection.clear();
	
	if ((err = cv2pprimg(frame, &image)) != PPR_SUCCESS) {
		cout << ppr_error_message(err) << endl;
		return;
	}
	
	if ((err = ppr_detect_faces(ppr_context, image, &face_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(err) << endl;
		return;
	}

	for (int i = 0; i < face_list.length; i++) {
		face = face_list.faces[i];
		if ((err = ppr_get_face_attributes(face, &face_attr)) != PPR_SUCCESS) {
			cout << ppr_error_message(err) << endl;
		}
		detection.push_back(face_attr);
	}

	ppr_free_face_list(face_list);
	ppr_free_image(image);
}

ppr_error_type cv2pprimg(const Mat &frame, ppr_image_type *image)
{
	ppr_raw_image_type raw_img;
	ppr_error_type err;
	raw_img.data = frame.data;
	raw_img.width = frame.cols;
	raw_img.height = frame.rows;
	raw_img.bytes_per_line = frame.step;
	raw_img.color_space = PPR_RAW_IMAGE_BGR24;

	err = ppr_create_image(raw_img, image);

	return err;
}

void FaceDetector::drawDetection(Mat &frame)
{
	vector<ppr_face_attributes_type>::iterator it;
	for (it = detection.begin(); it != detection.end(); it++) {
		ppr_face_attributes_type attr = *it;
		cv::circle(frame, Point(attr.position.x, attr.position.y), 3, Scalar(0, 255, 0), -1);
	}
}

bool ppr2cvimg(ppr_image_type *image, Mat &frame)
{
	ppr_error_type err;
	ppr_raw_image_type raw_image;
	if ((err = ppr_image_to_raw_image(*image, &raw_image)) != PPR_SUCCESS)
		return false;
	Mat temp = Mat(raw_image.height, raw_image.width, CV_8UC3, raw_image.data);
	temp.copyTo(frame);

	return true;
}