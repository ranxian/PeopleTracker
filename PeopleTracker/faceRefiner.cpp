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

	face_rect = Rect(attr.position.x - attr.dimensions.width / 2,
		attr.position.y - attr.dimensions.height / 2, attr.dimensions.width, attr.dimensions.height);

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
				Rect tracker_rect(r.xc - r.w / 2, r.yc - r.h / 2, r.w, r.h);
				Rect intersect = tracker_rect & face_rect;
				ratios[i] = intersect.area() / (double)face_rect.area();
				if (ratios[i] >= max_tracker_ratio)
					max_ratio_tracker_id = i;
			}
		}
	}
	
	// The one with maximum intersect ratio is the best asscociated tracker
	ppr_add_face(ppr_context, &trackers[max_ratio_tracker_id].gallery, face, max_ratio_tracker_id, trackers[max_ratio_tracker_id].faceCnt);
	trackers[max_ratio_tracker_id].faceCnt += 1;
}

void FaceRefiner::findTypycalFace(RefinerTracker *tracker)
{

}
void FaceRefiner::calcTrackerLinkNumber(RefinerTracker *tracker1, RefinerTracker *tracker2)
{

}
void FaceRefiner::mergeTrackers()
{

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
			trackers[result.id].lastFrameResult = result;
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

	for (int i = 0; i < MAX_REFINER_TRACKER_NUM; i++) {
		findTypycalFace(&trackers[i]);
	}
	
	mergeTrackers();
	
	outputResults();
}
