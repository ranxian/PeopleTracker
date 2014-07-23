enum MotionType
{
	PICK_UP,
	MOTION_SIZE
};

struct MotionInterval
{
	MotionType type;
	int s, t;
	int person;
	MotionInterval(MotionType _type, int _s, int _t,int _person = -1) :
		type(_type), s(_s), t(_t), person(_person)
	{}
};