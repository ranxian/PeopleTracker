#include <iostream>

#include "benchmark.h"
#include "parameter.h"
#include "util.h"
#include "dataReader.h"
#include "detector.h"
#include "tracker.h"
#include "multiTrackAssociation.h"
#include "xmlHelper.h"
#include "tinyxml2.h"

using namespace std;

void BenchmarkRunner::run()
{
	LOG_FACE_TO_TRACK_RATIO = 3;
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
	string resultPath = string(BDIR) + "\\" + testname + "-result.xml";
	if (!fexists(videoPath))
		cout << "video " + videoPath + " not exist" << endl;
	if (!fexists(xmlPath))
		cout << "xml file " << xmlPath << " not exist" << endl;
	
	// Run tracker
	runTracker(videoPath.c_str(), xmlPath.c_str(), resultPath.c_str());
}

// Run tracker for one instance
void BenchmarkRunner::runTracker(const char *videoFilePath, const char *xmlFilePath, const char *resultFilePath)
{
	VideoReader *reader = new VideoReader(videoFilePath);
	XMLDetector *detector = new XMLDetector(xmlFilePath);
	Mat frame;
	Mat drawFrame;
	FRAME_SIZE = reader->getFrameSize();
	TrakerManager mTrack(detector, frame, EXPERT_THRESH);
	mTrack.toggleDrawDetection();
	mTrack.toggleShowFace();

	FILE *file = fopen(resultFilePath, "w");
	int totalFrame = reader->getFrameCount();

	tinyxml2::XMLPrinter printer(file);
	printer.PushHeader(true, true);
	printer.OpenElement("dataset");

	reader->readImg(frame);
	for (int frameCnt = 0; frame.data != NULL; frameCnt++) {
		mTrack.doWork(frame);
		writeFrameToXml(printer, mTrack.getCurrentFrameResult());
		pyrDown(frame, drawFrame);
		imshow("s", drawFrame);
		
		printf("%d/%d\r", frameCnt, totalFrame);
		
		waitKey(10);
		reader->readImg(frame);
	}
	printer.CloseElement();
	fprintf(file, printer.CStr());

	delete reader;
	delete detector;
}
