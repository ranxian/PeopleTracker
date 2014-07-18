/*
 * Refined by Ran Xian in 2014
 */

/*************************************************************
*	Implemetation of the multi-person tracking system described in paper
*	"Online Multi-person Tracking by Tracker Hierarchy", Jianming Zhang,
*	Liliana Lo Presti, Stan Sclaroff, AVSS 2012
*	http://www.cs.bu.edu/groups/ivc/html/paper_view.php?id=268
*
*	Copyright (C) 2012 Jianming Zhang
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*	If you have problems about this software, please contact: jmzhang@bu.edu
*/

#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>
#include <Windows.h>

#include "tracker.h"
#include "detector.h"
#include "dataReader.h"
#include "multiTrackAssociation.h"
#include "parameter.h"

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

// Display set
int show_detection = 0;

// Heat map
int HEAT_RADIUS;

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
		reader=new VideoReader(_sequence_path_);
		break;
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

	TrakerManager mTrack(detector, frame, EXPERT_THRESH, result_output_xmlpath);
	VideoWriter writer;
	writer.open("Data\\result.mpg", CV_FOURCC('X', 'V', 'I', 'D'), 20, 
				Size(frame.cols, frame.rows));
	
	for (int frameCount=0;frame.data!=NULL;frameCount++)
	{
		mTrack.doWork(frame);	
		
		cout << "HERE" << endl;
		imshow("multiTrack", frame);

		reader->readImg(frame);
		writer.write(frame);

		char c = waitKey(1);
		if(c == 'q') break;
		else if (c=='p') {
			cvWaitKey(0);
		} else if (c == 'd') {
			show_detection = !show_detection;
		} else if(c != -1)
		{
			mTrack.setKey(c);
		}
	}

	delete reader;
	delete detector;
}

void playResult()
{
	SeqReader* reader;
	XMLBBoxReader boxReader(_result_xml_file_.c_str());
	vector<Result2D> result;

	reader = new VideoReader(_sequence_path_);
	cout << _sequence_path_ << endl;
	Mat frame;
	while (true) {
		reader->readImg(frame);
		if (frame.data == NULL)
			break;
		boxReader.getNextFrameResult(result);
		vector<Result2D>::iterator it;
		for (it = result.begin(); it != result.end(); it++) {
			Point p1((int)((*it).xc - (*it).w / 2), (int)((*it).yc - (*it).h / 2));
			Point p2((int)((*it).xc + (*it).w / 2), (int)((*it).yc + (*it).h / 2));
			rectangle(frame, p1, p2, COLOR((*it).id), 3);
		}
		imshow("Result", frame);
		Sleep(1000 / 35.0);
	}

	delete reader;
}

// Turn xx.mp4 to xx
string getBaseName(string videoName)
{
	char buf[256];
	strcpy_s(buf, videoName.c_str());
	strtok(buf, ".");
	char *bn = strtok(NULL, buf);
	if (bn == NULL)
		return "";
	else
		return String(buf);
}

int main(int argc,char** argv)
{
	cout << "1: Play Result, 2: Run" << endl;
	int option;
	cin >> option;

	if (option == 2) {
		cout << "Make sure the video is in the Data directory" << endl
			<< "And the detection xml is in the same directory and same base name" << endl
			<< "For example, if video named people.mp4 is given, a people.xml is " << endl
			<< "Expected." << endl;

		cout << "Enter the video name: ";

		string videoName;
		cin >> videoName;
		string baseName = getBaseName(videoName);
		_sequence_path_ = "Data\\" + videoName;
		_detection_xml_file_ = "Data\\" + baseName + ".xml";
		result_output_xmlpath = "Data\\" + baseName + "-result.xml";
		
		read_config();
		multiTrack(VIDEO, XML);
	} else {
		cout << "Enter video name: ";
		string videoName;
		cin >> videoName;
		_sequence_path_ = "Data\\" + videoName;
		_result_xml_file_ = "Data\\" + getBaseName(videoName) + "-result.xml";
		cout << _result_xml_file_ << endl;
		playResult();
	}

	return 0;
}
