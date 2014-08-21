#include "faceRefiner.h"
#include <iostream>
#include "util.h"
#include <direct.h>
#include <stdlib.h>
#include <stdio.h>
#include "utilities/sdk_utilities.h"

FaceRefiner::FaceRefiner(string seq_path_, string result_path_, string new_result_path_) :
videoReader(seq_path_), resultReader(result_path_.c_str()), new_result_path(new_result_path_), frameCnt(0), faceCnt(0)
{
	ppr_error_type r;
	if ((r = ppr_create_gallery(ppr_context, &gallery)) != PPR_SUCCESS) {
		cout << "FaceRefiner: " << ppr_error_message(r) << endl;
	}
	cout << "Has result? (y/n)" << endl;
	char ans;
	cin >> ans;
	if (ans == 'Y' || ans == 'y')
		hasResult = true;
	else
		hasResult = false;

	// Create data dir
	rootPath =  getBaseName(seq_path_);
	int re = _mkdir(rootPath.c_str());

	// Remove cluster dir
	clusterPath = rootPath + "\\" + "cluster";
	_rmdir(clusterPath.c_str());
}

void FaceRefiner::associateFace(ppr_face_type face)
{
	ppr_face_attributes_type attr; 
	ppr_error_type r;
	Rect face_rect;
	double ratios[REFINER_MAX_TRACKER_NUM] = {};

	if ((r = ppr_get_face_attributes(face, &attr)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
		return;
	}

	face_rect = faceBox2rect(&attr);

	// Add it as a detection
	Rect detect_rect = detector.guessPeopleDetection(face);
	weakDetectionInTheFrame.push_back(detect_rect);

	int max_ratio_tracker_id = -1;
	double max_tracker_ratio = 0;

	for (int i = 0; i < REFINER_MAX_TRACKER_NUM; i++) {
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
				Rect tracker_rect = box2rect(&r);
				Rect intersect = tracker_rect & face_rect;
				ratios[i] = (double)intersect.area() / (double)face_rect.area();
				if (ratios[i] >= max_tracker_ratio) {
					max_ratio_tracker_id = i;
					max_tracker_ratio = ratios[i];
				}
			}
		}
	}


	int subid;

	if (max_ratio_tracker_id >= 0 && max_tracker_ratio >= REFINER_FACE_ASSOC_THRES) {
		// The one with maximum intersect ratio is the best asscociated tracker
		cout << "Face associate to tracker #" << max_ratio_tracker_id << ", " << getGalleryFaceNum(gallery) << " faces in the gallery" << endl;
		subid = max_ratio_tracker_id;
	} else {
		cout << "Face has no assciated tracker" << endl;
		subid = REFINER_UNKOWN_SUBJECT_ID;
	}

	//if (subid != REFINER_UNKOWN_SUBJECT_ID) {
		faceRectInTheFrame.push_back(face_rect);
		assocInTheFrame.push_back(subid);
		if ((r = ppr_add_face(ppr_context, &gallery, face, subid, faceCnt)) != PPR_SUCCESS) {
			cout << "associateFace:ppr_add_face: " << ppr_error_message(r) << endl;
		}
		RefinerFace rface(face, faceCnt, subid);
		faceCnt += 1;
		allFaces.push_back(rface);
	//}
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
	ppr_similarity_matrix_ref_type smatrix_ref;
	ppr_id_list_type sublist;
	// Get similarity score

	if ((r = ppr_get_self_similarity_matrix_reference(ppr_context, gallery, &smatrix_ref)) != PPR_SUCCESS) {
		cout << "mergeTrackers:ppr_get_self_similarity_matrix_reference: " << ppr_error_message(r) << endl;
	}

	// Diff subjects

	if ((r = ppr_get_subject_id_list(ppr_context, gallery, &sublist)) != PPR_SUCCESS) {
		cout << "mergeTrackers:ppr_get_subject_id_list: " << ppr_error_message(r) << endl;
	}

	// Print subject templates
	int subject_temp_count[110] = {};
	for (int i = 0; i < allFaces.size(); i++) {
		for (int j = 0; j < sublist.length; j++) {
			if (sublist.ids[j] == allFaces[i].subjectID) {
				int hs = 0;
				if ((r = ppr_face_has_template(ppr_context, allFaces[i].face, &hs)) != PPR_SUCCESS) {
					cout << "mergeTrackers:ppr_face_has_template: " << ppr_error_message(r) << endl;
				}
				if (hs) {
					cout << "sub " << sublist.ids[j] << " has temp" << endl;
					subject_temp_count[sublist.ids[j]]++;
				}
			}
		}
	}

	for (int i = 0; i < sublist.length; i++) {
		cout << "Subject " << sublist.ids[i] << " has " << subject_temp_count[sublist.ids[i]] << " templates" << endl;
	}

	if ((r = ppr_get_subject_id_list(ppr_context, gallery, &id_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
	}

	for (int i = 0; i < id_list.length; i++) {
		if (id_list.ids[i] == REFINER_UNKOWN_SUBJECT_ID)
			continue;
		if ((r = ppr_set_subject_diff(ppr_context, &gallery, id_list.ids[i], REFINER_UNKOWN_SUBJECT_ID)) != PPR_SUCCESS) {
			cout << "mergeTrackers:ppr_set_subject_diff: " << ppr_error_message(r) << endl;
		}
	}


	if ((r = ppr_cluster_gallery(ppr_context, &gallery, 0, &cluster_list)) != PPR_SUCCESS) {
		cout << ppr_error_message(r) << endl;
	}

	cout << id_list.length << " subject merged into " << cluster_list.length << " clusters" << endl;

	// Print cluster result
	sdk_print_utils_print_cluster_list(cluster_list);

	// Apply merge result
	for (int i = 0; i < cluster_list.length; i++) {
		ppr_id_list_type clusteri = cluster_list.clusters[i];
		for (int j = 1; j < clusteri.length; j++) {
			merge(clusteri.ids[0], clusteri.ids[j]);
		}
	}

	// Calc link,
	// First get subject list
	/*
	if ((r = ppr_get_subject_id_list(ppr_context, gallery, &sublist)) != PPR_SUCCESS) {
		cout << "mergeTrackers:ppr_get_subject_id_list: " << ppr_error_message(r) << endl;
	}

	for (int i = 0; i < sublist.length - 1; i++) {
		for (int j = i + 1; j < sublist.length; j++) {
			int nLink = calcLink(sublist.ids[i], sublist.ids[j]);
			cout << "Subject " << sublist.ids[i] << " and subject " << sublist.ids[j] << " has " << nLink << " Links" << endl;
		}
	}
	*/
}	

// Merge results of clusteri and clusterj
void FaceRefiner::merge(int clusteri, int clusterj)
{
	RefinerTracker *cluster1 = &trackers[clusteri];
	RefinerTracker *cluster2 = &trackers[clusterj];

	for (int i = 0; i < cluster2->results.size(); i++) {
		if (cluster2->results[i].valid && !cluster1->results[i].valid) {
			cluster1->results[i] = cluster2->results[i];
		}
	}
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
	// Write xml result
	tinyxml2::XMLPrinter printer;

	printer.PushHeader(true, true);
	printer.OpenElement("dataset");

	for (int i = 0; i < frameCnt; i++) {
		vector<Result2D> results;
		for (int j = 0; j < REFINER_MAX_TRACKER_NUM; j++) {
			if (trackers[j].valid && trackers[j].results[i].valid) {
				results.push_back(trackers[j].results[i]);
			}
		}
		writeFrameToXml(printer, results);
	}

	printer.CloseElement();
	FILE *file = fopen(new_result_path.c_str(), "w");
	if (file == NULL) {
		cout << "Can't open file " << new_result_path << " for writing result" << endl;
	}
	fprintf(file, printer.CStr());

	//  Write cluster result
	sdk_utils_make_directory(clusterPath.c_str());
	sdk_utils_write_cluster_thumbnails(clusterPath.c_str(), ppr_context, gallery, cluster_list);
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

void FaceRefiner::drawTrackerWithFace()
{
	namedWindow("Tracker&Face");
	Mat drawFrame;
	frame.copyTo(drawFrame);
	// Draw tracker
	for (int i = 0; i < REFINER_MAX_TRACKER_NUM; i++) {
		if (trackers[i].valid) {
			Result2D result = trackers[i].results.back();
			Rect trect = box2rect(&result);
			putText(drawFrame, std::to_string(result.id), trect.tl(), FONT_HERSHEY_PLAIN, 3, COLOR(result.id));
			// Draw the tracker
			rectangle(drawFrame, trect, COLOR(result.id), 3);
		}
	}
	// Draw face
	for (int i = 0; i < faceRectInTheFrame.size(); i++) {
		rectangle(drawFrame, faceRectInTheFrame[i], COLOR(assocInTheFrame[i]), 3);
		putText(drawFrame, "Face #" + std::to_string(assocInTheFrame[i]), faceRectInTheFrame[i].tl(), FONT_HERSHEY_PLAIN, 3, COLOR(assocInTheFrame[i]));
	}
	// Draw weak detection
	for (int i = 0; i < weakDetectionInTheFrame.size(); i++) {
		rectangle(drawFrame, weakDetectionInTheFrame[i], Scalar(0, 0, 255), 3);
		putText(drawFrame, "Detection", weakDetectionInTheFrame[i].tl(), FONT_HERSHEY_PLAIN, 3, Scalar(0, 0, 255));
	}

	faceRectInTheFrame.clear();
	assocInTheFrame.clear();
	weakDetectionInTheFrame.clear();
	imshow("Tracker&Face", drawFrame);
}

// The overall binding
void FaceRefiner::solve()
{
	vector<Result2D> results;
	ppr_face_list_type face_list;
	vector<Result2D>::iterator it;
	ppr_error_type r;
	int nTemp = 0;

	while (true) {
		// Read frame
		videoReader.readImg(frame);
		if (frame.empty() || frame.data == NULL)
			break;
		frameCnt += 1;
		if (frameCnt < 100)
			continue;
		
		cout << "Frame #" << frameCnt << endl;

		// Read result for this frame
		resultReader.getNextFrameResult(results);

		cout << results.size() << " tracker in this frame" << endl;
		
		for (it = results.begin(); it != results.end(); it++) {
			Result2D result = *it;

			// Because we have used a compressed video
			int	scale = 5;
			result.xc = scale*(int)result.xc;
			result.yc = scale*(int)result.yc;
			result.w = scale*(int)result.w;
			result.h = scale*(int)result.h;

			trackers[result.id].results.push_back(result);

			if (trackers[result.id].valid == false)
				trackers[result.id].valid = true;
		}

		// Add invalid result for trackers with no results
		for (int i = 0; i < REFINER_MAX_TRACKER_NUM; i++) {
			if (trackers[i].results.size() < frameCnt) {
				Result2D result;
				result.valid = false;
				trackers[i].results.push_back(result);
			}
		}

		// Detect faces
		if (!hasResult) {
			detector.detect(frame);
			face_list = detector.getDetections();
			cout << face_list.length << " face detected" << endl;
			// Write face list to disk
			writeFaceList(frameCnt, face_list);
		} else {
			cout << "has result" << endl;
			readFaceList(frameCnt, &face_list);
		}

		for (int i = 0; i < face_list.length; i++) {
			ppr_image_type image;
			cv2pprimg(frame, &image);
			if ((r = ppr_extract_face_template(ppr_context, image, &face_list.faces[i])) != PPR_SUCCESS) {
				cout << "solve:ppr_extract_face_template: " << ppr_error_message(r) << endl;
			}
			int hs = 0;
			if ((r = ppr_face_has_template(ppr_context, face_list.faces[i], &hs)) != PPR_SUCCESS) {
				cout << "solve:ppr_face_has_template: " << ppr_error_message(r) << endl;
			}

			if (hs) {
				nTemp += 1;
			}

			associateFace(face_list.faces[i]);
			ppr_free_image(image);
		}

		drawTrackerWithFace();

		char key;
		key = waitKey(10);
		if (key == 'q')
			break;

		cout << "----------------------------------" << endl;
	}

	findTypycalFace();
	
	mergeTrackers();
	
	outputResults();

	cout << nTemp << " templates in total, " << allFaces.size() << " faces in total." << endl;
}

/// Read detected face for @frame
void FaceRefiner::readFaceList(int frame, ppr_face_list_type *face_list)
{
	ppr_error_type r;
	ppr_flat_data_type flat_data;

	char frameNum[20];
	_itoa(frame, frameNum, 10);
	string filename = rootPath + "\\" + string(frameNum) + ".facelist";

	if ((r = ppr_read_flat_data(ppr_context, filename.c_str(), &flat_data)) != PPR_SUCCESS) {
		cout << "readFaceList:ppr_read_flat_data: " << ppr_error_message(r) << endl;
	}
	if ((r = ppr_unflatten_face_list(ppr_context, flat_data, face_list)) != PPR_SUCCESS) {
		cout << "readFaceList:ppr_unflatten_face_list: " << ppr_error_message(r) << endl;
	}

	ppr_free_flat_data(flat_data);
}

void FaceRefiner::writeFaceList(int frame, ppr_face_list_type face_list)
{
	ppr_error_type r;
	ppr_flat_data_type flat_data;

	char frameNum[20];
	_itoa(frame, frameNum, 10);
	string filename = rootPath + "\\" + string(frameNum) + ".facelist";

	FILE *f = fopen(filename.c_str(), "w");
	fclose(f);

	if ((r = ppr_create_flat_data(sizeof(ppr_face_list_type), &flat_data)) != PPR_SUCCESS) {
		cout << "writeFaceList:ppr_create_flat_data: " << ppr_error_message(r) << endl;
	}

	if ((r = ppr_flatten_face_list(ppr_context, face_list, &flat_data)) != PPR_SUCCESS) {
		cout << "writeFaceList:ppr_flatten_face_list: " << ppr_error_message(r) << endl;
	}

	if ((r = ppr_write_flat_data(ppr_context, filename.c_str(), flat_data)) != PPR_SUCCESS) {
		cout << "writeFaceList:ppr_write_flat_data: " << ppr_error_message(r) << endl;
	}

	ppr_free_flat_data(flat_data);
}

int FaceRefiner::calcLink(int subid1, int subid2)
{
	int nLink = 0;
	// First create two gallery for the two subjects
	ppr_gallery_type gallery1;
	ppr_gallery_type gallery2;
	ppr_similarity_matrix_type sm_matrix;
	ppr_error_type r;

	if ((r = ppr_create_gallery(ppr_context, &gallery1)) != PPR_SUCCESS) {
		cout << "calcLink:ppr_create_gallery: " << ppr_error_message(r) << endl;
	}
	
	if ((r = ppr_create_gallery(ppr_context, &gallery2)) != PPR_SUCCESS) {
		cout << "calcLink:ppr_create_gallery: " << ppr_error_message(r) << endl;
	}

	addFacesToGallery(subid1, &gallery1);
	addFacesToGallery(subid2, &gallery2);
	// Then compare the two galleries
	if ((r = ppr_compare_galleries(ppr_context, gallery1, gallery2, &sm_matrix)) != PPR_SUCCESS) {
		cout << "calcLink:ppr_compare_galleries: " << ppr_error_message(r) << endl;
	}
	

	vector<int> sub1faces = getSubjectFaceIds(subid1);
	vector<int> sub2faces = getSubjectFaceIds(subid2);

	for (int i = 0; i < sub1faces.size(); i++) {
		for (int j = 0; j < sub2faces.size(); j++) {
			int faceid1 = sub1faces[i];
			int faceid2 = sub2faces[j];
			float score;
			if ((r = ppr_get_face_similarity_score(ppr_context, sm_matrix, faceid1, faceid2, &score)) != PPR_SUCCESS) {
				cout << "calcLink:ppr_get_face_similarity_score: " << ppr_error_message(r) << endl;
			}

			if (score > 0) {
				nLink += 1;
			}
		}
	}

	ppr_free_similarity_matrix(sm_matrix);
	ppr_free_gallery(gallery1);
	ppr_free_gallery(gallery2);

	return nLink;
}

// Add faces from this->gallery to @gallery
void FaceRefiner::addFacesToGallery(int subid, ppr_gallery_type *target_gallery)
{
	ppr_error_type r;

	for (int i = 0; i < allFaces.size(); i++) {
		RefinerFace rface = allFaces[i];
		if (rface.subjectID == subid) {
			if ((r = ppr_add_face(ppr_context, target_gallery, rface.face, rface.subjectID, rface.faceID)) != PPR_SUCCESS) {
				cout << "addFacesToGallery:ppr_add_face: " << ppr_error_message(r) << endl;
			}
		}
	}
}

vector<int> FaceRefiner::getSubjectFaceIds(int subid)
{
	vector<int> results;

	for (int i = 0; i < allFaces.size(); i++) {
		if (allFaces[i].subjectID == subid)
			results.push_back(allFaces[i].faceID);
	}

	return results;
}
