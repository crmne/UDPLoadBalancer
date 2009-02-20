#include <stdio.h>
#include <err.h>
void usage(char *program)
{
    printf("Usage: %s <infile> <outfile>\n", program);
}
int main(int argc, char *argv[])
{
    FILE *infile, *outfile;
    unsigned int currinfile[2], curr = 0, pktok = 0, pktloss =
        0, pktdelay = 0;
    int result, firsttime = 1;

    if (argc != 3) {
        usage(argv[0]);
        errx(1, "Wrong number of arguments");
    }

    infile = fopen(argv[1], "r");
    if (infile == NULL)
        err(2, "Unable to open %s", argv[1]);

    outfile = fopen(argv[2], "w");
    if (outfile == NULL)
        err(2, "Unable to open %s", argv[2]);
    do {
        result = fscanf(infile, "%u %u\n", &currinfile[0], &currinfile[1]);
        if (result < 0 && result != EOF)
            err(3, "fscanf()");
        if (firsttime) {
            curr = currinfile[0];
            firsttime = 0;
        } else
            curr++;
        while (curr < currinfile[0]) {
            fprintf(outfile, "%u %u\n", curr, 2);
            pktloss++;
            curr++;
        }
        if (currinfile[1] > 150) {
            pktdelay++;
            fprintf(outfile, "%u %u\n", curr, 1);
        } else {
            pktok++;
            fprintf(outfile, "%u %u\n", curr, 0);
        }
    } while (result != EOF);
    printf("Statistics for %s:\n\
    %u Total packets\n\
    %u received OK, %u delayed too much, %u were lost\n\
    %g%% packet loss\n", argv[1], pktok + pktdelay + pktloss, pktok, pktdelay, pktloss, ((double) pktdelay + (double) pktloss) / ((double) pktok + (double) pktdelay + (double) pktloss) * 100);
    return 0;
}
