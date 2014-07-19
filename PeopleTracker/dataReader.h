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


#ifndef DATA_READER_H
#define DATA_READER_H

#include <cstdio>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "tinyxml2.h"
#include "util.h"

using namespace cv;
using namespace std;
using namespace tinyxml2;
namespace txml = tinyxml2;

#define VIDEO 0
#define IMAGE 1

typedef struct Result2D
{
	int id;
	float xc, yc;  //center point
	float w, h; //width, height
	double response;
	Result2D(int i,float x_,float y_,float w_,float h_,double res=1)
		:id(i),xc(x_),yc(y_),w(w_),h(h_),response(res){}
	Result2D(){}
} Result2D;

// Sequence reader interface
class SeqReader
{
public:
	SeqReader(){};
	virtual void readImg(Mat& frame)=0;
	virtual ~SeqReader(){};
};

class BBoxReader // interface for reading bounding boxes from files
{
public:
	virtual bool getNextFrameResult(vector<Result2D>& result)=0;
};

class BBoxWriter // interface for reading bounding boxes from files
{
public:
	virtual bool putNextFrameResult(vector<Result2D>& result)=0;
};

/* ****** ****** */
// VideoReader - reads frames for a video
class VideoReader:public SeqReader
{
public:
	VideoReader(const string filename):capture(filename){}
	virtual void readImg(Mat& frame){ capture>>frame; }

private:
	VideoCapture capture;
};

// ImageDataReader - reads image under a dir
class ImageDataReader:public SeqReader
{
public:
	ImageDataReader(const string dir);
	virtual void readImg(Mat& frame);

private:
	// Number of files
	int _file_counter;
	// Directory path
	string _directory;
	// Image files under _directory
	vector<string> _m_fileNames;
};
// XMLBBoxReader - read boxes from a xml file
class XMLBBoxReader:public BBoxReader
{
public:
	XMLBBoxReader(const char* filename);
	// ~XMLBBoxReader() { }
	inline bool getOpenSuc(){return open_success;}
	// Return boxes in next frame
	virtual bool getNextFrameResult(vector<Result2D>& result);
private:
	txml::XMLDocument file;
	txml::XMLElement *frame;
	const char *temp; 
	bool open_success;
};
#define ENCODING "UTF-8"
// XMLBBoxWriter - write boxes to a xml files
class XMLBBoxWriter: public BBoxWriter
{
public:
	XMLBBoxWriter(const char* filename);

	// Put box in next frame
	virtual bool putNextFrameResult(vector<Result2D>& result);
	inline bool getOpenSuc(){return open_success;}
	~XMLBBoxWriter() 
	{
		cout << "XMLBoxWriter deconstructed" << endl;
		printer.CloseElement();
	}
private:
	txml::XMLPrinter printer;
	const char *temp;
	bool open_success;
	int frameCount;
	FILE *file;
};


#endif
