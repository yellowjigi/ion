#ifndef _TICTOC_H_
#define _TICTOC_H_

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PATH_ROOT	"/home/hoyeonjigi/ampsu_perf/"
#define PATH_INSTALL	"/home/hoyeonjigi/ampsu_perf/install/"
#define PATH_RESTART	"/home/hoyeonjigi/ampsu_perf/restart/"
#define PATH_REPORT	"/home/hoyeonjigi/ampsu_perf/report/"
#define PATH_FIN	"/home/hoyeonjigi/ampsu_perf/fin/"
//extern const char	*g_path[4];
//extern int	g_idx;
extern clock_t	g_begin[3];
extern clock_t	g_end[3];

#define CLOCK(prefix, path)\
	do {\
		char buf[64];\
		snprintf(buf, sizeof buf, prefix "%s", path);\
		double now = (double)clock() / CLOCKS_PER_SEC;\
		writeTime(now, buf);\
	} while(0)

#define CLOCK_INSTALL(path)\
	CLOCK(PATH_INSTALL, path)

#define CLOCK_RESTART(path)\
	CLOCK(PATH_RESTART, path)

#define CLOCK_REPORT(path)\
	CLOCK(PATH_REPORT, path)

#define CLOCK_FIN(path)\
	CLOCK(PATH_FIN, path)

#define CLOCK_BEGIN(depth)\
	do {\
		g_begin[depth] = clock();\
	} while(0)

#define CLOCK_END(depth, path)\
	do {\
		g_end[depth] = clock();\
		double elapsed = (double)(g_end[depth] - g_begin[depth]) / CLOCKS_PER_SEC;\
		char buf[64];\
		snprintf(buf, sizeof buf, PATH_ROOT "%s", path);\
		writeTime(elapsed, buf);\
	} while(0)

extern void writeTime(double time, char *path);

#endif //_TICTOC_H_
