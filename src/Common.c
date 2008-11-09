#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <err.h>
#include <sys/select.h>

void flushQueues()
{

}

void signalHandler(int signum)
{
	fprintf(stderr, "Received signal %d\n", signum);
	flushQueues();
	exit(0);
}

/** Configure signal handlers */
void configSigHandlers()
{
	struct sigaction act;
	sigemptyset(&act.sa_mask);
	act.sa_flags = SA_SIGINFO;	/* maybe not needed */
	act.sa_sigaction = signalHandler;

	if (sigaction(SIGINT, &act, NULL) < 0)
		errx(1, "failed to set SIGINT handler");
	if (sigaction(SIGTERM, &act, NULL) < 0)
		errx(1, "failed to set SIGTERM handler");

}
