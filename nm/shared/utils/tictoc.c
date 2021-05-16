#include "tictoc.h"

//const char	*g_path[4] = {
//	"/home/hoyeonjigi/ampsu_perf/install",
//	"/home/hoyeonjigi/ampsu_perf/restart",
//	"/home/hoyeonjigi/ampsu_perf/report",
//	"/home/hoyeonjigi/ampsu_perf/fin"
//};
//int	g_idx;
clock_t	g_begin[3];
clock_t	g_end[3];

void writeTime(double time, char *path)
{
	FILE *fp;

	printf("Path: %s.\n", path);//log

	if ((fp = fopen(path, "a")) == NULL)
	{
		fprintf(stderr, "fopen failed.\n");
		exit(EXIT_FAILURE);
	}

	fprintf(fp, "%lf\n", time);

	fclose(fp);
}
