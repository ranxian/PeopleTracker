/*
 * Refined by Ran Xian, xianran@pku.edu.cn
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
***************************************************************/

#include "detector.h"

// Draw detected object on @frame
void Detector::draw(Mat& frame)
{
	for (size_t i=0;i<detection.size();++i)
		// Draw a random color box
		rectangle(frame,detection[i],Scalar((double)((3*i)%256),(double)((57*i)%256),(double)((301*i)%256)));
}

/* ****** ****** */
// XMLDetector use detected result stored in a xml to detect object.
// The structure of the xml looks like:
// <dataset>
// 	 <frame>
//     <objectlist>
//		 <object confidance=""><box w="", h="", xc="", yc=""></box></object>
//     </objectlist>
//  </frame>
XMLDetector::XMLDetector(const char* filename):Detector(XML)
{
	int r = file.LoadFile(filename);
	// file=xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if (r != 0) {
		cout << "fail to open file: " << filename << endl;
		open_success = false;
	} else {
		open_success = true;
		frame = file.RootElement();
		if (frame == NULL) {
			cout << "empty file" << endl;
			open_success = false;
		}
		if (strcmp(frame->Name(), "dataset")) {
			cout << "bad file, root is not with name dataset" << endl;
			open_success = false;
		}
		else {
			frame = frame->FirstChildElement("frame");
		}
	}
}

void XMLDetector::detect(const Mat& f)
{
	if (!open_success)
		return;

	detection.clear();
	response.clear();
	bool r = false;
	Result2D res;
	Rect rectRes;
	if (frame != NULL) {
		r = true; //get the successive frame

		txml::XMLElement *objectList = frame->FirstChildElement("objectlist");
		if (objectList != NULL) {
			txml::XMLElement *object = objectList->FirstChildElement("object");
			float confidence = 0;
			if (object != NULL)
				confidence = string2float(object->Attribute("confidence"));
			while (object != NULL) //object level
			{
				Result2D res;
				temp = object->Attribute("id");
				res.id = string2int(temp);
				txml::XMLElement *box = object->FirstChildElement("box");

				if (box != NULL) {
					temp = box->Attribute("h");
					res.h = (float)string2float((char*)temp);
					temp = box->Attribute("w");
					res.w = (float)string2float((char*)temp);
					temp = box->Attribute("xc");
					res.xc = (float)string2float((char*)temp);
					temp = box->Attribute("yc");
					res.yc = (float)string2float((char*)temp);

					rectRes.height = cvRound(res.h);
					rectRes.width = cvRound(res.w);
					rectRes.x = cvRound(res.xc - 0.5*res.w);
					rectRes.y = cvRound(res.yc - 0.5*res.h);

					// Resize the detection
					for (int kk = 0; kk < LOG_FACE_TO_TRACK_RATIO - 1; kk++) {
						rectRes.x *= 2;
						rectRes.y *= 2;
						rectRes.height *= 2;
						rectRes.width *= 2;
					}

					if (rectRes.height / (double)rectRes.width >= 1.75 
						&& rectRes.height <= FRAME_SIZE.height*0.75
						&& rectRes.width <= FRAME_SIZE.width*0.5) {
						detection.push_back(rectRes);
						response.push_back(confidence);
					} else {
						Point wierd(rectRes.height, rectRes.width);
						cout << wierd << " is a wierd height width ratio, abort" << endl;
					}
				}
				object = object->NextSiblingElement("object");
			}
		}
		frame = frame->NextSiblingElement("frame");
	} else {
		cout << "No detections in the frame" << endl;
	}
}

/* ************ */

HogDetector::HogDetector():Detector(HOG),cpu_hog(Size(64,128), Size(16, 16), Size(8, 8), Size(8, 8), 9, 1, -1,
	HOGDescriptor::L2Hys, 0.2, false, cv::HOGDescriptor::DEFAULT_NLEVELS)
{
	detector = HOGDescriptor::getDefaultPeopleDetector();
	cpu_hog.setSVMDetector(detector);
}

void HogDetector::detect(const Mat& frame)
{
	cpu_hog.detectMultiScale(frame, detection, response, 0.0, Size(8,8),Size(0, 0), 1.05, 2);//-0.2
	for (vector<Rect>::iterator it=detection.begin(); it<detection.end(); it++)
	{
		// The scale is for finding the person box from the HOG detector output
		it->x=(int)(it->x / HOG_DETECT_FRAME_RATIO);
		it->y=(int)(it->y / HOG_DETECT_FRAME_RATIO);
		it->width=(int)(it->width / HOG_DETECT_FRAME_RATIO);
		it->height=(int)(it->height / HOG_DETECT_FRAME_RATIO);
	}
}
