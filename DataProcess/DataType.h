#ifndef DATATYPE_H
#define DATATYPE_H
#include<vector>

const float invalid_value = 1e30f;

struct NodeType
{
	float x,y,z;
	int r;
	NodeType(float _x = 0.f, float _y = 0.f, float _z = 0.f) :
		x(_x), y(_y), z(_z)
	{}
	NodeType operator - (const NodeType &a) const
	{
		return NodeType(x - a.x, y - a.y, z - a.z);
	}
	float dis() const
	{
		return (float)sqrt(x*x + y*y + z*z);
	}
	float operator * (const NodeType &a) const
	{
		return x * a.x + y*a.y + z*a.z;
	}
	float angle(NodeType a, NodeType b) const
	{
		NodeType e1 = a - *this, e2 = b - *this;
		return (float)acos(e2 * e1 / (e1.dis() * e2.dis()));
	}
};

struct PeopleType
{
	const static int node_number = 20;
	NodeType node[node_number];
	bool tracked;
};

struct FrameType
{
	std::vector<PeopleType> people;
};
#endif