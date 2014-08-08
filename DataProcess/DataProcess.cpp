#include "DataProcess.h"
#include <cmath>
#include <algorithm>

float calc_right_elbow_angle(const PeopleType &people)
{
	int r = people.node[9].r + people.node[8].r + people.node[10].r;
	if (r >= 6)
	{
		return people.node[9].angle(people.node[8], people.node[10]);
	}
	else
	{
		return invalid_value;
	}
}

float calc_left_elbow_angle(const PeopleType &people)
{
	int r = people.node[5].r + people.node[4].r + people.node[6].r;
	if (r >= 6)
	{
		return people.node[5].angle(people.node[4], people.node[6]);
	}
	else
	{
		return invalid_value;
	}
}


struct Feature
{
	float left_elbow, right_elbow, distance;
};

struct FeatureInfo
{
	int illegalFrames, startFrame;
};

vector< vector<Feature> > get_feature_list(const vector<FrameType> &frames)
{
	vector< vector<Feature> > featureList;
	vector< const PeopleType *> lastValidFrame;

	int maxPeople = 0;
	for (auto p = frames.begin(); p != frames.end(); ++p)
	{
		maxPeople = max(maxPeople, (int)p->people.size());
	}

	for (int i = 0; i < maxPeople; ++i)
	{
		lastValidFrame.push_back(nullptr);
	}

	for (auto p = frames.begin(); p != frames.end(); ++p)
	{
		vector<Feature> features;
		for (size_t i = 0; i<p->people.size();++i)
		{
			const auto &person = p->people[i];
			Feature feature;
			feature.left_elbow = calc_left_elbow_angle(person);
			feature.right_elbow = calc_right_elbow_angle(person);
			auto q = p;
			float vMin = invalid_value;
			if (p->people[i].tracked)
			{
				for (int count = 0; q >= frames.begin() && count < 10; q--, count++)
				if (q->people.size() > i && q->people[i].tracked)
				{
					NodeType nodeDiff = p->people[i].node[0] - q->people[i].node[0];
					nodeDiff.z = 0;
					vMin = min(vMin, nodeDiff.dis());
				}
			}
			feature.distance = vMin;
			features.push_back(feature);
		}
		featureList.push_back(features);
	}
	return featureList;
}


vector<MotionInterval> DataProcess::analyze_data(const vector<FrameType> &frames)
{
	vector<MotionInterval> ret;
	auto featureList = get_feature_list(frames);
	
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
			if ((feature.left_elbow < threshold || feature.right_elbow < threshold) && feature.distance < position_threshold)
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
					if (feature.distance < position_threshold)
					{
						info.illegalFrames++;
					}
					else
					{
						info.illegalFrames = ignore_frames;
					}
				}
			}
		}
		
	}
	return ret;
}

