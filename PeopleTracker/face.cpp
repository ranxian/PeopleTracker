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

bool init_ppr_sdk()
{
	ppr_error_type r;

	if ((r = ppr_initialize_sdk("models", my_license_id, my_license_key)) != PPR_SUCCESS) {
		const char *msg = ppr_error_message(r);
		cout << msg << endl;
		return false;
	} else {
		cout << "PittPatt SDK initialized. Configuring..." << endl;
		ppr_settings_type setting = ppr_get_default_settings();
		setting.detection.enable = 1;
		setting.detection.use_serial_face_detection = 1;
		setting.detection.search_pruning_aggressiveness = 1;
		setting.detection.threshold = 0.5;
		setting.detection.num_threads = sdk_utils_get_num_cpus();
		setting.recognition.enable_extraction = 1;
		setting.recognition.enable_comparison = 1;
		setting.detection.extract_thumbnails = 1;
		setting.recognition.num_comparison_threads = sdk_utils_get_num_cpus();
		setting.recognition.automatically_extract_templates = 1;
		setting.landmarks.enable = 1;
		setting.landmarks.manually_detect_landmarks = 0;
		if ((r = ppr_initialize_context(setting, &ppr_context)) != PPR_SUCCESS) {
			cout << ppr_error_message(r) << endl;
			return false;
		} else {
			cout << "PittPatt SDK context successfully set" << endl;
		}
	}
	return true;
}

void finalize_sdk() {
	ppr_finalize_context(ppr_context);
	ppr_finalize_sdk();
}

FaceDetector::FaceDetector()
{
	detected = false;
}

FaceDetector::~FaceDetector()
{
	if (detected) {
		ppr_free_face_list(face_list);
	}
}

void FaceDetector::detect(const Mat &frame)
{
	ppr_image_type image;
	ppr_error_type err;

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

Rect FaceDetector::guessPeopleDetection(ppr_face_type face, double *conf)
{
	Rect face_rect;
	ppr_face_attributes_type attr;
	ppr_error_type r;
	if ((r = ppr_get_face_attributes(face, &attr)) != PPR_SUCCESS) {
		cout << "guessPeopleDetection:ppr_get_face_attributes: " << ppr_error_message(r) << endl;
	}
	face_rect = faceBox2rect(&attr);
	Rect detect_rect = face_rect;
	detect_rect.x -= face_rect.width;
	detect_rect.y -= 10;
	detect_rect.height *= 6.5;
	detect_rect.width *= 3;

	if (conf != NULL)
		*conf = attr.confidence;

	return detect_rect;
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

ppr_face_list_type FaceDetector::getDetections() {
	return face_list;
}

int getGalleryFaceNum(ppr_gallery_type gallery)
{
	ppr_error_type r;
	ppr_id_list_type ilist;
	if ((r = ppr_get_face_id_list(ppr_context, gallery, &ilist)) != PPR_SUCCESS) {
		cout << "getGalleryFaceNum:ppr_get_face_id_list: " << ppr_error_message(r) << endl;
		return -1;
	} else {
		return ilist.length;
	}
}

Rect faceBox2rect(const ppr_face_attributes_type *attr) {
	return Rect((int)(attr->position.x - attr->dimensions.width / 2), (int)(attr->position.y - attr->dimensions.height / 2),
		(int)attr->dimensions.width, (int)attr->dimensions.height);
}