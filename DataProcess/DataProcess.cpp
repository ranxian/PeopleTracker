#include "DataProcess.h"
#include <cmath>

float calc_right_elbow_angle(const PeopleType &people)
{
	return people.node[9].angle(people.node[8], people.node[10]);
}

float calc_left_elbow_angle(const PeopleType &people)
{
	return people.node[5].angle(people.node[4], people.node[6]);
}

struct Feature
{
	float left_elbow, right_elbow;
};

struct FeatureInfo
{
	int illegalFrames, startFrame;
};


vector<MotionInterval> DataProcess::analyze_data(const vector<FrameType> &frames)
{
	vector<MotionInterval> ret;
	vector< vector<Feature> > featureList;
	for (auto p = frames.begin(); p != frames.end(); ++p)
	{
		vector<Feature> features;
		for (auto q = p->people.begin(); q != p->people.end(); ++q)
		{
			Feature feature;
			feature.left_elbow = calc_left_elbow_angle(*q);
			feature.right_elbow = calc_right_elbow_angle(*q);
			features.push_back(feature);
		}
		featureList.push_back(features);
	}
	
	vector<FeatureInfo> infoList;
	
	for (size_t i = 0; i != featureList.size(); ++i)
	{
		const auto & vec = featureList[i];
		for (size_t j = 0; j != vec.size(); ++j)
		{
			while (j >= infoList.size())
			{
				FeatureInfo info;
				info.illegalFrames = ignore_frames;
				infoList.push_back(info);
			}

			auto &feature = vec[j];
			auto &info = infoList[j];
			if (feature.left_elbow < threshold || feature.right_elbow < threshold)
			{
				if (info.illegalFrames == ignore_frames)
				{
					info.startFrame = i;
				}
				if (info.illegalFrames > 0)
				{
					info.illegalFrames--;
				}
			}
			else
			{
				if (info.illegalFrames < ignore_frames )
				{
					if (info.illegalFrames == ignore_frames - 1 && (int)i - info.startFrame - ignore_frames >= min_frames)
					{
						ret.push_back(MotionInterval(MotionType::PICK_UP, info.startFrame, i));
					}
					info.illegalFrames++;
				}
			}
		}
		
	}
	return ret;
}

