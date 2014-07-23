#include "DataScanner.h"

std::vector<FrameType> DataScanner::scan(const std::string &path)
{
	FILE *fin = nullptr;

	const int max_buf = 1000;
	char buf[max_buf];
	fopen_s(&fin, path.c_str(), "r");
	std::vector<FrameType> ret;
	while (1)
	{
		FrameType frame;

		if (fgets(buf, max_buf, fin) == nullptr)
			break;
		fgets(buf, max_buf, fin);
		while (strcmp("\n", buf))
		{
			PeopleType people;
			for (int i = 0; i < PeopleType::node_number; ++i)
			{
				NodeType node;
				sscanf_s(buf, "%f%f%f", &node.x, &node.y, &node.z);
				people.node[i] = node;
				fgets(buf, max_buf, fin);
			}
			frame.people.push_back(people);
		}
		ret.push_back(frame);
	}
	fclose(fin);
	return ret;
}