#ifndef _FACE_REFINER_
#define _FACE_REFINER_
#include "face.h"
#include "tracker.h"
#define REFINER_MAX_TRACKER_NUM 20
#define REFINER_FACE_ASSOC_THRES 0.8
#define REFINER_REP_FACE_NUM 5
#define REFINER_CLUSTER_AGGR 6
#define REFINER_UNKOWN_SUBJECT_ID 100
class RefinerTracker
{
public:
	RefinerTracker() : updated(false), valid(false) {};
	vector<Result2D> results;
	bool updated;
	bool valid;
};

class RefinerFace
{
public:
	RefinerFace(ppr_face_type face_, int faceID_, int subjectID_) : face(face_), faceID(faceID_), subjectID(subjectID_) {};
	ppr_face_type face;
	int faceID;
	int subjectID;
};

class FaceRefiner
{
public:
	FaceRefiner(string seq_path_, string result_path_, string new_result_path_, bool benchmarking=false);
	FaceRefiner::~FaceRefiner();
	void solve();
	bool benchmarking;
	bool hasResult;
private:
	void merge(int clusteri, int clusterj);
	void readFaceList(int frame, ppr_face_list_type *face_list);
	void writeFaceList(int frame, ppr_face_list_type face_list);
	void printGalleryFaceNum();
	void drawTrackerWithFace();
	// Use tracker id to find tracker
	RefinerTracker trackers[REFINER_MAX_TRACKER_NUM];
	void associateFace(ppr_face_type face);
	void findTypycalFace();
	void mergeTrackers();
	void outputResults();
	int calcLink(int subid1, int subid2);
	void addFacesToGallery(int subid, ppr_gallery_type *target_gallery);
	vector<int> FaceRefiner::getSubjectFaceIds(int subid);
	VideoReader videoReader;
	XMLBBoxReader resultReader;
	string new_result_path;
	FaceDetector detector;
	ppr_cluster_list_type cluster_list;
	ppr_gallery_type gallery;
	int faceCnt;

	int frameCnt;

	Mat frame;

	vector<Rect> faceRectInTheFrame;
	vector<int> assocInTheFrame;
	vector<Rect> weakDetectionInTheFrame;
	vector<Result2D> weakResults;

	vector<RefinerFace> allFaces;

	string rootPath;
	string clusterPath;
};

#endif