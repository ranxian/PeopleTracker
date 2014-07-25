#include "face.h"
#include "stdio.h"
#include "utilities/sdk_draw_utils.h"

IplImage *convert_ppr_image_to_opencv(ppr_raw_image_type *p_image)
{
	IplImage *cv_image;
	unsigned char *data;
	int i, j, k;

	ppr_raw_image_convert(p_image, PPR_RAW_IMAGE_BGR24);
	cv_image = cvCreateImage(cvSize(p_image->width, p_image->height), IPL_DEPTH_8U, 3);

	data = (unsigned char*)cv_image->imageData;

	for (i = 0; i<p_image->height; i++)
	for (j = 0; j<p_image->width; j++)
	for (k = 0; k<3; k++)
		data[i * cv_image->widthStep + j * 3 + k] =
		p_image->data[i * p_image->bytes_per_line + j * 3 + k];

	return cv_image;
}

ppr_raw_image_type convert_opencv_image_to_ppr(IplImage *cv_image)
{
	ppr_raw_image_type p_image;
	unsigned char *data;
	int i, j, k;

	ppr_raw_image_create(&p_image, cv_image->width, cv_image->height, PPR_RAW_IMAGE_BGR24);

	data = (unsigned char*)cv_image->imageData;

	for (i = 0; i<p_image.height; i++)
	for (j = 0; j<p_image.width; j++)
	for (k = 0; k<3; k++)
		p_image.data[i * p_image.bytes_per_line + j * 3 + k] =
		data[i * cv_image->widthStep + j * 3 + k];

	return p_image;
}


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

	detected = false;
}

FaceDetector::~FaceDetector()
{
	if (detected) {
		ppr_free_face_list(face_list);
	}
	ppr_finalize_context(ppr_context);
	ppr_finalize_sdk();
}

void FaceDetector::detect(const Mat &frame)
{
	ppr_image_type image;
	ppr_error_type err;
	ppr_face_attributes_type face_attr;
	ppr_face_type face;

	if (detected) {
		ppr_free_face_list(face_list);
		detected = false;
	}
	
	if ((err = cv2pprimg(frame, &image)) != PPR_SUCCESS) {
		cout << ppr_error_message(err) << endl;
		return;
	}
	
	if ((err = ppr_detect_faces(ppr_context, image, &face_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(err) << endl;
		return;
	}

	detected = true;
	ppr_free_image(image);
}


ppr_error_type cv2pprimg(const Mat &frame, ppr_image_type *image)
{
	ppr_raw_image_type raw_img;
	ppr_error_type err;
	IplImage iplimg = frame;
	raw_img = convert_opencv_image_to_ppr(&iplimg);

	err = ppr_create_image(raw_img, image);

	return err;
}

void FaceDetector::drawDetection(Mat &frame)
{
	vector<ppr_face_type>::iterator it;
	ppr_raw_image_type raw_image;
	ppr_image_type image;
	ppr_image_type temp_image;
	sdk_draw_utils_line_attributes_type line_attr;
	ppr_face_attributes_type face_attr;
	line_attr.color = SDK_DRAW_UTILS_GREEN;
	line_attr.line_type = SDK_DRAW_UTILS_SOLID_LINE;
	line_attr.thickness = 3;

	cv2pprimg(frame, &image);
	ppr_image_to_raw_image(image, &raw_image);

	sdk_draw_utils_overlay_face_list(&raw_image, face_list, 1);

	ppr_create_image(raw_image, &temp_image);
	ppr2cvimg(&temp_image, frame);

	ppr_free_image(temp_image);
	ppr_free_image(image);
}

bool ppr2cvimg(ppr_image_type *image, Mat &frame)
{
	ppr_error_type err;
	ppr_raw_image_type raw_image;
	if ((err = ppr_image_to_raw_image(*image, &raw_image)) != PPR_SUCCESS)
		return false;

	IplImage *p_iplimg;

	p_iplimg = convert_ppr_image_to_opencv(&raw_image);

	frame = Mat(p_iplimg);

	return true;
}