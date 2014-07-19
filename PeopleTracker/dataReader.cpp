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
#include "OS_specific.h"
#include "dataReader.h"

// ImageDataReader - Read images in the @dir
ImageDataReader::ImageDataReader(const string dir):_directory(dir),_file_counter(0)
{
	char tmpDirSpec[MAX_PATH+1];
	sprintf_s(tmpDirSpec, "%s*", _directory.c_str());

#if OS_type==2 // for windows
	WIN32_FIND_DATAA f;
	HANDLE h = FindFirstFileA(tmpDirSpec , &f); // read .
	if(h != INVALID_HANDLE_VALUE)
	{
		FindNextFileA(h, &f);	//read ..
		while(FindNextFileA(h, &f))
			_m_fileNames.push_back(f.cFileName);
	}
	FindClose(h);
#endif

#if OS_type==1 // for linux
	DIR *dp;
	struct dirent *dirp;
	if((dp = opendir(dir.c_str())) == NULL) {
		cout << "Error opening " << dir << endl;
	}

	readdir(dp);//.
	readdir(dp);//..
	while ((dirp = readdir(dp)) != NULL) {
		string filename(dirp->d_name);
		if (
			filename.find(".jpg")!=string::npos ||
			filename.find(".jpeg")!=string::npos ||
			filename.find(".png")!=string::npos ||
			filename.find(".bmp")!=string::npos
			) {
			_m_fileNames.push_back(filename);	
		}
	}
	std::sort(_m_fileNames.begin(),_m_fileNames.end());
	closedir(dp);
#endif
}
void ImageDataReader::readImg(Mat& frame)
{
	frame.data=NULL;
	if (_file_counter>=_m_fileNames.size())
		return;
	frame=imread(_directory+_m_fileNames[_file_counter]);
	_file_counter++;
}

/* ****** ****** */

XMLBBoxReader::XMLBBoxReader(const char* filename)
{
	int r = file.LoadFile(filename);
	// file=xmlReadFile(filename,"UTF-8",XML_PARSE_RECOVER);
	if (r != 0)
	{
		cout<<"fail to open"<<endl;
		open_success = false;
	}
	else
	{
		open_success = true;
		frame = file.RootElement();
		if (frame == NULL)
		{
			cout << "empty file" << endl;
			open_success = false;
		}
		if (strcmp(frame->Name(), "dataset"))
		{
			cout << "bad file, root is not with name dataset" << endl;
			open_success = false;
		}
		else
		{
			frame = frame->FirstChildElement("frame");
		}
	}			
}
bool XMLBBoxReader::getNextFrameResult(vector<Result2D>& result)
{
	bool r = false;
	result.clear();
	if (frame != NULL)
	{
		r = true; //get the successive frame

		txml::XMLElement *objectList = frame->FirstChildElement("objectlist");
		if (objectList != NULL)
		{
			txml::XMLElement *object = objectList->FirstChildElement("object");
			while (object!=NULL) //object level
			{
				Result2D res;
				temp = object->Attribute("id");
				res.id=string2int(temp);
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
					result.push_back(res);
				}
				object = object->NextSiblingElement("object");
			}
		}

		frame = frame->NextSiblingElement("frame");
	}
	
	return r;
}	

/* ****** ****** */

XMLBBoxWriter::XMLBBoxWriter(const char* filename):frameCount(0)
{
	cout << "what" << endl;
	fopen_s(&file, filename, "w");
	if (file == NULL) {
		cout << "can't open file " << filename << " for write" << endl;
	} else {
		open_success = true;
		printer = txml::XMLPrinter(file);
		printer.PushHeader(true, true);
		printer.OpenElement("dataset");
	}
}
bool XMLBBoxWriter::putNextFrameResult(vector<Result2D>& result)
{
	// return true;
	printer.OpenElement("frame");
	printer.PushAttribute("number", frameCount);
	printer.OpenElement("objectlist");
	vector<Result2D>::iterator it;

	for (it=result.begin();it<result.end();it++)
	{
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
	return true;
}