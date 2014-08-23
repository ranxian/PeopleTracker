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
#include "faceRefiner.h"


string BenchmarkRunner::getGoldPath(const char *testname)
{
	return string(BDIR) + "\\" + testname + "-gold.txt";
}
string BenchmarkRunner::getVideoPath(const char *testname)
{
	return string(BDIR) + "\\" + testname + ".avi";
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
	LOG_FACE_TO_TRACK_RATIO = 2;

	cout << "Tracker location test" << endl;
	// Test location
	testLocation();
	cout << "-----------------" << endl << "Tracker staying time test" << endl;
	// Test staying
	testStay();
}

void BenchmarkRunner::testLocation()
{
	bool has_result;
	// First ask if result exist
	cout << "has result? (y/n)" << endl;
	char h;
	cin >> h;

	if (h == 'y' || h == 'Y') {
		has_result = true;
	} else {
		has_result = false;
	}

	if (!has_result) {
		for (int i = 0; i < nLocTest; i++) {
			char *testname = locTestList[i];
			cout << "Test: " << testname << endl;

			// Run tracker
			runTracker(testname);
		}
	}

	getLocationScore();
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

			//imshow("benchmark", frame);
			//waitKey(0);
		}

		fclose(file);
	}
}

void BenchmarkRunner::testStay()
{
	cout << "Tracker location test" << endl;

	bool has_result;
	// First ask if result exist
	cout << "has result? (y/n)" << endl;
	char h;
	cin >> h;

	if (h == 'y' || h == 'Y') {
		has_result = true;
	} else {
		has_result = false;
	}

	//if (!has_result) {
		for (int ntest = 0; ntest < nStayTest; ntest++) {
			char *testname = stayTestList[ntest];
			cout << "Test: " << testname << endl;
			runTracker(testname);
		}
	//}
}

void BenchmarkRunner::getStayScore()
{

}

// Run tracker for one instance
void BenchmarkRunner::runTracker(const char *testname)
{
	string videoPath = getVideoPath(testname);
	string xmlPath = getDetectPath(testname);
	string resultPath = getResultPath(testname);
	if (!fexists(videoPath))
		cout << "video " + videoPath + " not exist" << endl;
	if (!fexists(xmlPath))
		cout << "xml file " << xmlPath << " not exist" << endl;

	// 1. Do multi track, use face detection to help
	cout << "Running tracker..." << endl;
	VideoReader *reader = new VideoReader(videoPath);
	XMLDetector *detector = new XMLDetector(xmlPath.c_str());
	Mat frame;
	Mat drawFrame;
	Mat initFrame;
	FRAME_SIZE = reader->getFrameSize();
	

	FILE *file = fopen(resultPath.c_str(), "w");
	int totalFrame = reader->getFrameCount();

	tinyxml2::XMLPrinter printer(file);
	printer.PushHeader(true, true);
	printer.OpenElement("dataset");

	reader->readImg(frame);
	cv::namedWindow("tracker");

	frame.copyTo(initFrame);
	for (int k = 0; k < LOG_FACE_TO_TRACK_RATIO - 1; k++)
		pyrDown(initFrame, initFrame);
	TrakerManager mTrack(detector, frame, EXPERT_THRESH);
	mTrack.toggleDrawDetection();
	mTrack.toggleShowFace();
	for (int frameCnt = 0; frame.data != NULL; frameCnt++) {
		mTrack.doWork(frame);
		writeFrameToXml(printer, mTrack.getCurrentFrameResult());
		//pyrDown(frame, drawFrame);
		//imshow("tracker", frame);

		printf("%d/%d\r", frameCnt, totalFrame);

		
		waitKey(10); 
		reader->readImg(frame);
	}
	printer.CloseElement();
	fprintf(file, printer.CStr());
	fclose(file);

	cv::destroyWindow("tracker");
	delete reader;
	delete detector;

	// 2. Smooth the image using python script
	

	// 3. Use face refiner
	cout << "Running face refiner..." << endl;
	FaceRefiner refiner(videoPath, resultPath, resultPath, true);
	refiner.benchmarking = true;
	refiner.hasResult = false;
	refiner.solve();

	// 4. Smooth again
	cout << "Running smoother..." << endl;
	// system((string("python") + " " + PYPATH + " " + resultPath + " " + resultPath).c_str());

	// 5. Show final result for tracker
	playResult(videoPath, resultPath, 1);
}

// Play results for all benchmarks, this can help for human to get score
void BenchmarkRunner::playBenchmarkResult()
{
	for (int i = 0; i < nLocTest; i++) {
		cout << i << ". " << locTestList[i] << endl;
	}

	for (int i = 0; i < nStayTest; i++) {
		cout << i + nLocTest << ". " << stayTestList[i] << endl;
	}

	cout << "Which?" << endl;
	int no;
	cin >> no;
	char *testname;
	if (no >= nLocTest) {
		testname = stayTestList[no - nLocTest];
	} else {
		testname = locTestList[no];
	}
}