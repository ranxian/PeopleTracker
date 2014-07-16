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
/* This source deals with data association problem */

#ifndef MULTI_TRACK_ASSOCIATION
#define MULTI_TRACK_ASSOCIATION

#include <fstream>

#include "opencv2/opencv.hpp"

#include "parameter.h"
#include "dataReader.h"
#include "util.h"
#include "tracker.h"
#include "detector.h"
#include "heatmap.h"
#include "face.h"

#define GOOD 0
#define NOTSURE 1
#define BAD 2

#define COUNT_NUM 1000.0
#define SLIDING_WIN_SIZE 7.2*TIME_WINDOW_SIZE 

using namespace cv;

// Wating list
class WaitingList
{
	typedef struct Waiting
	{
		int accu;
		Rect currentWin;
		Point center;
		int life_count;
		Waiting(Rect win)
			:accu(1),
			life_count(1),
			currentWin(win),
			center((int)(win.x+0.5*win.width),(int)(win.y+0.5*win.height)) {};
	} Waiting;

	list<Waiting> w_list;
	int life_limit;

public:
	WaitingList(int life):life_limit(life){}
	void update();
	vector<Rect>outputQualified(double thresh);
	void feed(Rect bodysize_win,double response);
};

class Controller
{
public:
	WaitingList waitList;
	WaitingList waitList_suspicious;

	Controller(
		Size sz,int r, int c,double vh=0.01,
		double lr=1/COUNT_NUM,
		double thresh_expert=0.5);
	void takeVoteForHeight(Rect bodysize_win);	
	vector<int> filterDetection(vector<Rect> detction_bodysize);	
	void takeVoteForAvgHittingRate(list<EnsembleTracker*> _tracker_list);	

	/*
	Tracker death control. For modifying termination conditions, change here.
	*/
	void deleteObsoleteTracker(list<EnsembleTracker*>& _tracker_list);	
	
	void calcSuspiciousArea(list<EnsembleTracker*>& _tracker_list);	
	inline vector<Rect> getQualifiedCandidates()
	{
		/*
		For modifying the birth condition for trackers, change here.
		*/
		double l=_hit_record._getAvgHittingRate(_alpha_hitting_rate,_beta_hitting_rate);
		return waitList.outputQualified((l-sqrt(l)-1.0));		
	}
private:
	// a rotate array for keep record of the average hitting rate
	typedef struct HittingRecord
	{
		Mat record;
		int idx;
		HittingRecord():idx(0)
		{
			record=Mat::zeros(2,(int)(SLIDING_WIN_SIZE),CV_64FC1);
		}
		void recordVote(bool vote)
		{
			idx=idx-record.cols*(idx/record.cols);
			record.at<double>(0,idx)=vote ? 1.0:0;
			record.at<double>(1,idx)=1;
			idx++;
		}
		double _getAvgHittingRate(double _alpha_hitting_rate, double _beta_hitting_rate)
		{
			Scalar s1=sum(record.row(0));
			Scalar s2=sum(record.row(1));
			return (s1[0]*TIME_WINDOW_SIZE+_alpha_hitting_rate)/(_beta_hitting_rate+s2[0]);
		}
	}HittingRecord;

	double _thresh_for_expert;
	
	Size _frame_size;
	int _grid_rows;
	int _grid_cols;
	double _prior_height_variance;
	vector<vector<double> > _bodyheight_map;
	vector<vector<double> > _bodyheight_map_count;
	double _bodyheight_learning_rate;

	HittingRecord _hit_record;
	double _alpha_hitting_rate;
	double _beta_hitting_rate;

	vector<Rect> _suspicious_rect_list;
};

class TrakerManager
{
public:
	TrakerManager(
		Detector* detctor,Mat& frame,
		double thresh_promotion);
	~TrakerManager();	
	void doWork(Mat& frame);

	void setKey(char c)
	{
		_my_char = c;
	}	
private:	
	void doHungarianAlg(const vector<Rect>& detections);
	inline static bool compareTraGroup(EnsembleTracker* c1,EnsembleTracker* c2)
	{
		return c1->getTemplateNum()>c2->getTemplateNum() ? true:false;
	}

	Controller _controller;
	Mat* _frame_set;
	list<EnsembleTracker*> _tracker_list;
	int _tracker_count;
	char _my_char;		
	
	Detector* _detector;
	int _frame_count;
	
	Mat _occupancy_map;	
	XMLBBoxWriter resultWriter;

	double _thresh_for_expert_;
	// Background substraction
	Mat fgMaskMog;
	Ptr<BackgroundSubtractorMOG2> pMog;

	// Heatmap
	Heatmap heatmap;

	// Face Detector
	FaceDetector face_detector;
};
	


#endif