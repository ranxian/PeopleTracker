#include <cstdio>
#include <vector>
#include <cmath>
#include "MotionInterval.h"
#include "DataType.h"
using namespace std;

namespace DataProcess
{
	const int fps = 30;
	const int ignore_frames = 5, min_frames = 15;
	const double threshold = acos(-1.) * 220 / 360;

	vector<MotionInterval> analyze_data(const vector<FrameType> &);
};