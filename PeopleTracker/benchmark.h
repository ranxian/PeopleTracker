#ifndef __BENCHMARK_H_
#define __BENCHMARK_H_

#include <string>

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
	void testLocation(const char *testname);
	void runTracker(const char *videoFilePath, const char *xmlFilePath);
};

#endif