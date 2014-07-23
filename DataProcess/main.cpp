#include "DataProcess.h"
#include "DataScanner.h"

int main()
{
	auto frames = DataScanner::scan("data.txt");
	auto motions = DataProcess::analyze_data(frames);
	FILE *fout;
	fopen_s(&fout, "motion.txt", "w");
	for (const auto &motion : motions)
	{
		fprintf(fout, "%d %d\n", motion.s, motion.t);
	}
}