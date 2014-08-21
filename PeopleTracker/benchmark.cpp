#include <iostream>
#include <cstdio>

#include "benchmark.h"
#include "parameter.h"
#include "util.h"
#include "dataReader.h"
#include "detector.h"
#include "tracker.h"
#include "multiTrackAssociation.h"
#include "xmlHelper.h"
#include "tinyxml2.h"


string BenchmarkRunner::getGoldPath(const char *testname)
{
	return string(BDIR) + "\\" + testname + "-gold.txt";
}
string BenchmarkRunner::getVideoPath(const char *testname)
{
	return string(BDIR) + "\\" + XMLPFX + testname + ".avi";
}
string BenchmarkRunner::getDetectPath(const char *testname)
{
	return string(BDIR) + "\\" + XMLPFX + testname + ".xml";
}
string BenchmarkRunner::getResultPath(const char *testname)
{
	return string(BDIR) + "\\" + testname + "-result.xml";
}

void BenchmarkRunner::run()
{
	LOG_FACE_TO_TRACK_RATIO = 3;
	cout << "Tracker location test" << endl;

	// First ask if result exist
	cout << "has result? (y/n)" << endl;
	char h;
	cin >> h;

	if (h == 'y' || h == 'Y') {
		has_result = true;
	} else {
		has_result = false;
	}

	// Test location
	testLocation();
	cout << "----" << endl << "Tracker staying time test" << endl;
	// Test staying
}

void BenchmarkRunner::testLocation()
{
	if (!has_result) {
		for (int i = 0; i < nLocTest; i++) {
			char *testname = locTestList[i];
			cout << "Test: " << testname << endl;
			string videoPath = getVideoPath(testname);
			string xmlPath = getDetectPath(testname);
			string resultPath = getResultPath(testname);
			if (!fexists(videoPath))
				cout << "video " + videoPath + " not exist" << endl;
			if (!fexists(xmlPath))
				cout << "xml file " << xmlPath << " not exist" << endl;

			// Run tracker
			runTracker(videoPath.c_str(), xmlPath.c_str(), resultPath.c_str());
		}
	}

	getLocationScore();
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
		//pyrDown(frame, drawFrame);
		//imshow("s", drawFrame);

		printf("%d/%d\r", frameCnt, totalFrame);

		//waitKey(10);
		reader->readImg(frame);
	}
	printer.CloseElement();
	fprintf(file, printer.CStr());
	fclose(file);

	delete reader;
	delete detector;
}

// Get tracker location score
void BenchmarkRunner::getLocationScore()
{
	for (int ntest = 0; ntest < nLocTest; ntest++) {
		cout << "Getting score for " << locTestList[ntest] << endl;
		char *testname = locTestList[ntest];
		string goldPath = getGoldPath(testname);
		string resultPath = getResultPath(testname);
		string videoPath = getVideoPath(testname);
		Mat frame;
		XMLBBoxReader reader(resultPath.c_str());
		VideoCapture cap(videoPath);
		int frameno;
		int nAnnoatete;

		FILE *file = fopen(goldPath.c_str(), "r");

		if (file == NULL) {
			cout << "Gold file not exist" << endl;
			return;
		}

		// Read groundtruth from gold data
		while ((fscanf(file, "%d %d", &frameno, &nAnnoatete)) != EOF) {
			int truth[10];
			vector<Result2D> results;
			cout << frameno << " " << nAnnoatete << endl;
			for (int i = 0; i < nAnnoatete; i++) {
				fscanf(file, "%d", &truth[i]);
			}
			
			// Read video
			cap.set(CV_CAP_PROP_POS_FRAMES, (double)frameno);
			cap.read(frame);

			// Find results for @frameno
			reader.getResultForFrame(results, frameno);
			for (Result2D r2d : results) {
				Rect rect = box2rect(&r2d);
				rectangle(frame, rect, Scalar(255, 0, 0), 3);
				putText(frame, std::to_string(r2d.id), rect.tl(), CV_FONT_HERSHEY_COMPLEX, 0.5, Scalar(255, 255, 255));
			}

			imshow("benchmark", frame);
			waitKey(0);
		}

		fclose(file);
	}
}