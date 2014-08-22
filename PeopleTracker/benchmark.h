#ifndef __BENCHMARK_H_
#define __BENCHMARK_H_

#include <string>
using namespace std;

#define BDIR "benchmark"
#define XMLPFX "s-"
#define PYPATH "benchmark\\process_result.py"

static char *locTestList[] = {
	"location-one-1"
};
static int nLocTest = 1;

static char *stayTestList[] = {
	"staying-1"
};
static int nStayTest = 1;

class BenchmarkRunner
{
public:
	void run();
private:
	void testLocation();
	void testStay();
	void runTracker(const char *testname);
	void getLocationScore();
	void getStayScore();
	std::string getGoldPath(const char *testname);
	std::string getVideoPath(const char *testname);
	std::string getResultPath(const char *testname);
	string getDetectPath(const char *testname);
};

#endif