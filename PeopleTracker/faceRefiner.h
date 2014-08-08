#ifndef _FACE_REFINER_
#define _FACE_REFINER_
#include "face.h"
#include "tracker.h"

class RefinerTracker
{
public:
	vector<Result2D> results;
	Result2D lastFrameResult;
	ppr_gallery_type gallery;
	ppr_gallery_type typycalFaces;
};

class FaceRefiner
{
public:
	FaceRefiner(string seq_path_, string result_path_, string new_result_path_) :
		videoReader(seq_path_), resultReader(result_path_.c_str()), new_result_path(new_result_path_){}
	void solve();
private:
	// Use tracker id to find tracker
	map<int, RefinerTracker*> trackers;
	void associateFace(ppr_face_type face);
	void findTypycalFace(RefinerTracker *tracker);
	void calcTrackerLinkNumber(RefinerTracker *tracker1, RefinerTracker *tracker2);
	void mergeTrackers();
	void outputResults();
	VideoReader videoReader;
	XMLBBoxReader resultReader;
	string new_result_path;
	FaceDetector detector;
};

#endif