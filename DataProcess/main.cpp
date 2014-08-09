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
		fprintf(fout, "%4d %4d:%4.2f %4.2f\n", motion.s, motion.t, motion.s/30., motion.t/30.);
	}
}