#include "faceRefiner.h"
#include <iostream>

void FaceRefiner::associateFace(ppr_face_type face)
{
	ppr_face_attributes_type attr; 
	ppr_error_type r;
	Rect face_rect;
	double ratios[MAX_REFINER_TRACKER_NUM] = {};

	if ((r = ppr_get_face_attributes(face, &attr)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
		return;
	}

	face_rect = Rect((int)attr.position.x - (int)(attr.dimensions.width / 2),
		(int)attr.position.y - (int)(attr.dimensions.height / 2), (int)attr.dimensions.width, (int)attr.dimensions.height);

	int max_ratio_tracker_id = -1;
	double max_tracker_ratio = 0;

	for (int i = 0; i < MAX_REFINER_TRACKER_NUM; i++) {
		if (trackers[i].valid) {
			Result2D r;
			r.valid = false;
			// Find most recenct 5 results
			for (int j = 0; j < 5; j++) {
				if (frameCnt - j - 1 < 0)
					break;
				if (trackers[i].results[frameCnt - j - 1].valid) {
					r = trackers[i].results[frameCnt-j-1];
					break;
				}
			}
			if (r.valid) {
				Rect tracker_rect((int)r.xc - (int)(r.w / 2), (int)r.yc - (int)(r.h / 2), (int)r.w, (int)r.h);
				Rect intersect = tracker_rect & face_rect;
				ratios[i] = intersect.area() / (double)face_rect.area();
				if (ratios[i] >= max_tracker_ratio)
					max_ratio_tracker_id = i;
			}
		}
	}

	if (max_ratio_tracker_id >= 0) {
		// The one with maximum intersect ratio is the best asscociated tracker
		cout << "Face associate to tracker #" << max_ratio_tracker_id << ", " << getGalleryFaceNum(gallery) << " faces in the gallery" << endl;
	} else {
		cout << "Face has no assciated tracker" << endl;
	}
}

void FaceRefiner::findTypycalFace()
{
	ppr_error_type r;

	if ((r = ppr_trim_subjects_to_representative_faces(ppr_context, &gallery, REFINER_REP_FACE_NUM)) != PPR_SUCCESS) {
		cout << "findTypycalFace: " << ppr_error_message(r) << endl;
		return;
	} 

	cout << "Typycal face found, trimed to " << getGalleryFaceNum(gallery) << endl;
}

void FaceRefiner::mergeTrackers()
{
	// Now faces are associated to trackers, we do a clustering directly on the gallery,
	// hopefully to merge different trackers for one person
	ppr_error_type r;
	ppr_id_list_type id_list;

	if ((r = ppr_get_subject_id_list(ppr_context, gallery, &id_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
	}

	if ((r = ppr_cluster_gallery(ppr_context, &gallery, 1, &cluster_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
	}

	cout << id_list.length << " subject merged into " << cluster_list.length << " clusters" << endl;
}


static void writeFrameToXml(tinyxml2::XMLPrinter &printer, vector<Result2D> &results)
{
	static int frameCount = 0;
	printer.OpenElement("frame");
	printer.PushAttribute("number", frameCount);
	printer.OpenElement("objectlist");

	vector<Result2D>::iterator it;
	for (it = results.begin(); it != results.end(); it++) {
		printer.OpenElement("object");
		printer.PushAttribute("id", (*it).id);
		printer.PushAttribute("confidence", (*it).response);

		printer.OpenElement("box");
		printer.PushAttribute("h", (*it).h);
		printer.PushAttribute("w", (*it).w);
		printer.PushAttribute("xc", (*it).xc);
		printer.PushAttribute("yc", (*it).yc);
		printer.CloseElement(); // end box

		printer.CloseElement(); // end object
	}

	printer.CloseElement(); // end objectlist
	printer.CloseElement(); // end frame
	frameCount++;
}

void FaceRefiner::outputResults()
{
	FILE *file = fopen(new_result_path.c_str(), "w");
	if (file == NULL) {
		cout << "Can't open file " << new_result_path << " for writing result" << endl;
	}

	tinyxml2::XMLPrinter printer(file);

	printer.PushHeader(true, true);
	printer.OpenElement("dataset");

	for (int i = 0; i < frameCnt; i++) {
		vector<Result2D> results;
		for (int j = 0; j < MAX_REFINER_TRACKER_NUM; j++) {
			if (trackers[i].valid && trackers[i].results[i].valid) {
				results.push_back(trackers[j].results[i]);
			}
		}
		writeFrameToXml(printer, results);
	}

	printer.CloseElement();
	fprintf(file, printer.CStr());
}

void FaceRefiner::printGalleryFaceNum()
{
	ppr_error_type r;
	ppr_id_list_type ilist;
	ilist.length = 0;
	if ((r = ppr_get_face_id_list(ppr_context, this->gallery, &ilist)) != PPR_SUCCESS) {
		cout << "associateFace: " << ppr_error_message(r) << endl;
	}
	cout << ilist.length << " face in the gallery" << endl;
}

FaceRefiner::~FaceRefiner()
{
	ppr_free_gallery(gallery);
	ppr_free_cluster_list(cluster_list);
}

// The overall binding
void FaceRefiner::solve()
{
	vector<Result2D> results;
	ppr_face_list_type face_list;
	vector<Result2D>::iterator it;
	Mat frame;
	while (true) {
		// Read frame
		videoReader.readImg(frame);
		if (frame.empty())
			break;
		frameCnt += 1;
		if (frameCnt == 10)
			break;
		// Read result for this frame
		resultReader.getNextFrameResult(results);
		for (it = results.begin(); it != results.end(); it++) {
			Result2D result = *it;
			trackers[result.id].results.push_back(result);
			if (trackers[result.id].valid == false)
				trackers[result.id].valid = true;
		}
		// Add invalid result for trackers with no results
		for (int i = 0; i < MAX_REFINER_TRACKER_NUM; i++) {
			if (trackers[i].results.size() < frameCnt) {
				Result2D result;
				result.valid = false;
				trackers[i].results.push_back(result);
			}
		}
		// Detect faces
		detector.detect(frame);
		face_list = detector.getDetections();
		cout << face_list.length << " face detected" << endl;
		for (int i = 0; i < face_list.length; i++) {
			associateFace(face_list.faces[i]);
		}
	}

	findTypycalFace();
	
	mergeTrackers();
	
	outputResults();
}
