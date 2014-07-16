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
***************************************************************/
/*
USAGE:

	a.
	Hierarchy_Ensemble <path_of_sequence> <is_image>
	b.
	Hierarchy_Ensemble <path_of_sequence> <is_image> <path_detection_file>
	c.
	Hierarchy_Ensemble <path_of_result> 2

	<path_of_seuqence>: the path of the directory containing images or the path of the video
	<is_image>: '1' for image format sequence, '0' the video format sequence
	<path_detection_file>: the xml detection file, whose structure should follow the example in
	the supplementary files

	When <detection_file> is not specified, the program will use the HOG detector to detect
	pedestrians online; Otherwise, the xml file specified will be read to get the detection results in
	it. For the exact structure of the detection file, see the example files in the supplementary files.

	You may need to change the parameters stored in 'config.txt'. There are detailed explanations and
	recommended values in it.


EXAMPLE:

	a.
	Hierarchy_Ensemble C:/video_data/TownCentreXVID.avi 0
	(The program will use the OpenCV's HOG detector)
	b.
	Hierarchy_Ensemble C:/video_data/PETS09S2L1/ 1 C:/PETS09_S2L1_det_opencv.xml
	(The program will read the images stored in the directory and use the detection xml file as the
	source of detections)


NOTE:

	(*) When the program is running, type 'p' to pause and 'q' to quit.
	(**) The tracking result will be recorded in a file named "output.xml".
*/

#include <ctime>
#include <iostream>
#include <sstream>
#include <fstream>

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

//Configuration
int MAX_TRACKER_NUM;
int MAX_TEMPLATE_SIZE;
int EXPERT_THRESH;
double BODYSIZE_TO_DETECTION_RATIO;
double TRACKING_TO_BODYSIZE_RATIO;
int FRAME_RATE;
double TIME_WINDOW_SIZE;
double HOG_DETECT_FRAME_RATIO;
int PLAY_RESULT;

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

	TrakerManager mTrack(detector,frame,EXPERT_THRESH);
	XMLBBoxReader boxReader(_result_xml_file_.c_str());
	vector<Result2D> result;
	for (int frameCount=0;frame.data!=NULL;frameCount++)
	{
		if (PLAY_RESULT) {
			boxReader.getNextFrameResult(result);
			vector<Result2D>::iterator it;
			// Draw it
			for (it = result.begin(); it != result.end(); it++) {
				Point p1((int)((*it).xc-(*it).w/2), (int)((*it).yc-(*it).h/2));
				Point p2((int)((*it).xc+(*it).w/2), (int)((*it).yc+(*it).h/2));
				rectangle(frame, p1, p2, COLOR((*it).id), 3);
			}
			int n = 10000000;
			while(n--);
		} else {
			mTrack.doWork(frame);	
		}
		
		imshow("multiTrack", frame);

		reader->readImg(frame);

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

void help()
{
	cout<<"usage: \n\n"
		"1.\n"
		"Hierarchy_Ensemble <sequence_path> <is_image>\n"
		"(by default, it uses hog detector in opencv to detect pedestrians)\n\n"

		"2.\n"
		"Hierarchy_Ensemble <sequence_path> <is_image> <detection_xml_file_path>\n"
		"(it uses detection stored in the specified xml file. You may rescale the detection bounding box "
		"by tuning parameters in the \"he_config.txt\")\n\n"

		"<is_image>: \'1\' for image format data. \'0\' for video format data.\n";
	getchar();
}

int main2(int argc, char **argv)
{
	cout << "Video name is " << argv[1] << endl;
	VideoCapture cap(argv[1]);
	Mat frame;

	int nframe = (int)cap.get(CV_CAP_PROP_FRAME_COUNT);
	int width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	int height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);
	cout << "number of frame: " << nframe << endl;
	cout << "Width: " << width << endl << "Height: " << height << endl;
	for (int i = 0; i < nframe; i++) {
		cap.read(frame);
		cout << i << endl;
		imshow("Frame", frame);
	}
}

int main(int argc,char** argv)
{
	cout << "You have to put video and detection xml file in the Data directory" << endl
		<< "and make the video and xml file with the same base name." << endl 
		<< "Enter the video name and prefix to run the program:" << endl;
	
	string videoName;
	string prefix;

	cin >> videoName >> prefix;

	_sequence_path_ = "Data\\" + videoName + prefix;
	_detection_xml_file_ = "Data\\" + videoName + ".xml";

	read_config();

	int seq_format;

	PLAY_RESULT = 0;

	seq_format = VIDEO;

	multiTrack(seq_format, XML);

	return 0;
}
