#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>

#include "tracker.h"
#include "detector.h"
#include "dataReader.h"
#include "multiTrackAssociation.h"
#include "parameter.h"
#include "faceRefiner.h"
#include "benchmark.h"

using namespace cv;
using namespace std;

static string _sequence_path_;
static string _detection_xml_file_;
static string _result_xml_file_;
static string result_output_xmlpath;

//Configuration
int MAX_TRACKER_NUM;
int MAX_TEMPLATE_SIZE;
int EXPERT_THRESH;
double BODYSIZE_TO_DETECTION_RATIO;
double TRACKING_TO_BODYSIZE_RATIO;
int FRAME_RATE;
double TIME_WINDOW_SIZE;
double HOG_DETECT_FRAME_RATIO;
Size FRAME_SIZE;
// Display set
int show_detection = 0;

// Heat map
int HEAT_RADIUS;

ppr_context_type ppr_context;

void read_config()
{
	ifstream conf_file("config.txt");

	if (!conf_file.is_open())
	{
		cerr<<"fail to load config.txt."<<endl;
		exit(1);
	}

	string line;
	while (conf_file.good())
	{
		getline(conf_file,line);
		istringstream line_s(line);
		string field;
		line_s>>field;
		if (field.compare("MAX_TRACKER_NUM:")==0)
			line_s>>MAX_TRACKER_NUM;
		else if (field.compare("FRAME_RATE:")==0)
			line_s>>FRAME_RATE;
		else if (field.compare("TIME_WINDOW_SIZE:")==0)
			line_s>>TIME_WINDOW_SIZE;
		else if (field.compare("HOG_DETECT_FRAME_RATIO:")==0)
			line_s>>HOG_DETECT_FRAME_RATIO;
		else if (field.compare("MAX_TEMPLATE_SIZE:")==0)
			line_s>>MAX_TEMPLATE_SIZE;
		else if (field.compare("EXPERT_THRESH:")==0)
			line_s>>EXPERT_THRESH;
		else if (field.compare("BODYSIZE_TO_DETECTION_RATIO:")==0)
			line_s>>BODYSIZE_TO_DETECTION_RATIO;
		else if (field.compare("TRACKING_TO_BODYSIZE_RATIO:")==0)
			line_s>>TRACKING_TO_BODYSIZE_RATIO;
		else if (field.compare("HEAT_RADIUS:")==0)
			line_s>>HEAT_RADIUS;
	}
	conf_file.close();
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

void multiTrack(int readerType,int detectorType)
{
	namedWindow("multiTrack",CV_WINDOW_AUTOSIZE);
	SeqReader* reader;
	Mat frame;
	switch (readerType)
	{
	case IMAGE:
		reader=new ImageDataReader(_sequence_path_);
		break;
	case VIDEO:
	{
				  reader = new VideoReader(_sequence_path_);
				  VideoCapture cap(_sequence_path_);
				  FRAME_SIZE = Size((int)cap.get(CV_CAP_PROP_FRAME_WIDTH), (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT));
				  break;
	}
	default:
		cerr<<"no such reader type!"<<endl;
		return ;
	}
	reader->readImg(frame);
	if (frame.data==NULL)
	{
		cerr<<"fail to open pictures!"<<endl;
		return ;
	}

	Detector* detector;
	switch (detectorType)
	{
	case HOG:
		detector=new HogDetector();
		break;
	case XML:
		detector=new XMLDetector(_detection_xml_file_.c_str());
		break;
	default:
		detector=new HogDetector();
		break;
	}

	TrakerManager mTrack(detector, frame, EXPERT_THRESH);
	FILE *file = fopen(result_output_xmlpath.c_str(), "w");
	
	tinyxml2::XMLPrinter printer(file);
	
	printer.PushHeader(true, true);
	printer.OpenElement("dataset");

	for (int frameCount=0;frame.data!=NULL;frameCount++)
	{
		mTrack.doWork(frame);	
		
		imshow("multiTrack", frame);

		writeFrameToXml(printer, mTrack.getCurrentFrameResult());
		reader->readImg(frame);

		char c = waitKey(1);
		if(c == 'q') break;
		else if (c=='p') {
			cvWaitKey(0);
		} else if (c == 'd') {
			show_detection = !show_detection;
		} else if (c != -1) {
			mTrack.setKey(c);
			cout << "set " << c << endl;
		}
	}

	printer.CloseElement();
	fprintf(file, printer.CStr());
	delete reader;
	delete detector;
}

void playResult()
{
	XMLBBoxReader boxReader(_result_xml_file_.c_str());
	vector<Result2D> result;
	VideoCapture cap(_sequence_path_);
	int width = cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	Mat frame;
	Mat heatImg;
	Heatmap hmap(height, width);
	namedWindow("Result");
	namedWindow("Heatmap");
	while (true) {
		cap.read(frame);
		if (frame.empty())
			break;
		boxReader.getNextFrameResult(result);
		vector<Result2D>::iterator it;
		for (it = result.begin(); it != result.end(); it++) {
			Point p1((int)((*it).xc - (*it).w / 2), (int)((*it).yc - (*it).h / 2));
			Point p2((int)((*it).xc + (*it).w / 2), (int)((*it).yc + (*it).h / 2));
			rectangle(frame, p1, p2, COLOR((*it).id), 3);
		}
		hmap.feed(result);
		hmap.drawHeatImg(frame);
		imshow("Result", frame);
		char key;
		key = waitKey(60);
		if (key == 'q')
			break;
		switch (key) {
		case 'p':
			while (waitKey(0) != 'p');
		default:
			break;
		}
	}
}

int main(int argc,char** argv)
{
	cout << "1: Play Result, 2: Run, 3: Face refine, 4: Benchmark" << endl;
	int option;
	cin >> option;

	read_config();

	if (!init_ppr_sdk()) {
		cout << "can't init pitt patt, quit" << endl;
		exit(-1);
	}

	if (option == 2) {
		cout << "Make sure the video is in the Data directory" << endl
			<< "And the detection xml is in the same directory and same base name" << endl
			<< "For example, if video named people.mp4 is given, a people.xml is " << endl
			<< "Expected." << endl;

		cout << "Enter the video name: ";

		string videoName;
		cin >> videoName;
		string baseName = getBaseName(videoName);
		_sequence_path_ = "tracker\\" + videoName;
		_detection_xml_file_ = "tracker\\" + baseName + ".xml";
		result_output_xmlpath = "tracker\\" + baseName + "-result.xml";
		
		multiTrack(VIDEO, XML);
	} else if (option == 1) {
		cout << "Enter video name: ";
		string videoName;
		cin >> videoName;
		_sequence_path_ = "tracker\\" + videoName;
		_result_xml_file_ = "tracker\\" + getBaseName(videoName) + "-result.xml";
		cout << _result_xml_file_ << endl;
		playResult();
	} else if (option == 3) {
		cout << "Enter video name: ";
		string videoName;
		cin >> videoName;
		_sequence_path_ = "tracker\\" + videoName;
		_result_xml_file_ = "tracker\\" + getBaseName(videoName) + "-result.xml";
		string new_result_xml_path = "tracker\\" + getBaseName(videoName) + "-result-new.xml";
		FaceRefiner refiner(_sequence_path_, _result_xml_file_, new_result_xml_path);
		refiner.solve();
	} else if (option == 4) {
		BenchmarkRunner runner;
		runner.run();
	}

	finalize_sdk();

	return 0;
}
