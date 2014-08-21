#ifndef __BENCHMARK_H_
#define __BENCHMARK_H_

#include <string>
using namespace std;

#define BDIR "benchmark"
#define XMLPFX "s-"

static char *locTestList[] = {
	"location-one-1"
};
static int nLocTest = 1;

class BenchmarkRunner
{
public:
	void run();
private:
	void testLocation();
	void runTracker(const char *videoFilePath, const char *xmlFilePath, const char *resultFilePath);
	void getLocationScore();
	std::string getGoldPath(const char *testname);
	std::string getVideoPath(const char *testname);
	std::string getResultPath(const char *testname);
	string getDetectPath(const char *testname);
	bool has_result;
};

#endif