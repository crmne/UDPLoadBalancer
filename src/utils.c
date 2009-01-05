#include "utils.h"
double calcDelay(struct timeval older)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	return ((double) (now.tv_sec - older.tv_sec) +
		(double) (now.tv_usec - older.tv_usec) / 1000000.0) * 1000;
}
