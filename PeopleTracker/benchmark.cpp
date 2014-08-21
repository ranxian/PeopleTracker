#include <iostream>

#include "benchmark.h"
#include "parameter.h"
#include "util.h"
#include "dataReader.h"
#include "detector.h"
#include "tracker.h"
#include "multiTrackAssociation.h"
#include "xmlHelper.h"

using namespace std;

void BenchmarkRunner::run()
{
	cout << "Tracker location test" << endl;

	// Test location
	for (int i = 0; i < nLocTest; i++) {
		testLocation(locTestList[i]);
	}

	cout << "----" << endl << "Tracker staying time test" << endl;

	// Test staying
}

void BenchmarkRunner::testLocation(const char *testname)
{
	cout << "Test: " << testname << endl;
	string videoPath = string(BDIR) + "\\" + testname + ".avi";
	string xmlPath = string(BDIR) + "\\" + XMLPFX + testname + ".xml";
	if (!fexists(videoPath))
		cout << "video " + videoPath + " not exist" << endl;
	if (!fexists(xmlPath))
		cout << "xml file " << xmlPath << " not exist" << endl;
	
	// Run tracker
	runTracker(videoPath.c_str(), xmlPath.c_str());
}

void BenchmarkRunner::runTracker(const char *videoFilePath, const char *xmlFilePath)
{
	VideoReader *reader = new VideoReader(videoFilePath);
	XMLDetector *detector = new XMLDetector(xmlFilePath);
	Mat frame;
	// TrackerManager mTrack(detector, frame, EXPERT_THRESH);
}
