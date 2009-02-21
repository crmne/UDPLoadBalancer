#include <stdio.h>
#include <stdint.h>
#include "macro.h"
int main(int argc, char *argv[])
{
    int i;
    packet_t one, two, three, four, five, six, seven, eight;
    packet_t *Queue = NULL;

    one.id = 1;
    two.id = 2;
    three.id = 3;
    four.id = 4;
    five.id = 5;
    six.id = 6;
    seven.id = 7;
    eight.id = 8;

    printQueue(Queue);

    fprintf(stderr, "Testing insertions...\n");
    insertInQ(&Queue, &four);
    printQueue(Queue);
    insertInQ(&Queue, &two);
    printQueue(Queue);
    insertInQ(&Queue, &three);
    printQueue(Queue);
    insertInQ(&Queue, &eight);
    printQueue(Queue);
    insertInQ(&Queue, &one);
    printQueue(Queue);
    insertInQ(&Queue, &seven);
    printQueue(Queue);

    fprintf(stderr, "Testing removes of first element...\n");
    for (i = 0; i < 10; i++) {
        getFirstInQ(&Queue);
        printQueue(Queue);
    }

    fprintf(stderr, "Repopulating queue...\n");
    insertInQ(&Queue, &four);
    insertInQ(&Queue, &two);
    insertInQ(&Queue, &three);
    insertInQ(&Queue, &eight);
    insertInQ(&Queue, &one);
    insertInQ(&Queue, &seven);
    printQueue(Queue);

    fprintf(stderr, "Testing general removes...\n");
    removeFromQ(&Queue, four.id);
    printQueue(Queue);
    removeFromQ(&Queue, two.id);
    printQueue(Queue);
    removeFromQ(&Queue, three.id);
    printQueue(Queue);
    removeFromQ(&Queue, five.id);
    printQueue(Queue);
    removeFromQ(&Queue, eight.id);
    printQueue(Queue);
    removeFromQ(&Queue, one.id);
    printQueue(Queue);
    removeFromQ(&Queue, seven.id);
    printQueue(Queue);

    return 0;
}
