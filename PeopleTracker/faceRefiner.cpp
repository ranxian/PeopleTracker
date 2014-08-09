#include "faceRefiner.h"
#include <iostream>

void FaceRefiner::associateFace(ppr_face_type face)
{

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
void FaceRefiner::outputResults()
{

}

// The overall binding
void FaceRefiner::solve()
{
	vector<Result2D> results;
	ppr_face_list_type face_list;
	vector<Result2D>::iterator it;
	Mat frame;
	int frameCnt = 0;
	while (true) {
		// Read frame
		videoReader.readImg(frame);
		if (frame.empty())
			break;
		frameCnt += 1;
		// Read result for this frame
		resultReader.getNextFrameResult(results);
		for (it = results.begin(); it != results.end(); it++) {
			Result2D result = *it;
			trackers[result.id].results.push_back(result);
			trackers[result.id].lastFrameResult = result;
			if (trackers[result.id].valid == false)
				trackers[result.id].valid = true;
		}
		// Detect faces
		detector.detect(frame);
		face_list = detector.getDetections();
		cout << face_list.length << " face detected" << endl;
	}
}
