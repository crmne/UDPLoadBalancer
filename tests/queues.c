#include <stdio.h>
#include <stdint.h>
#include "macro.h"
void q_print(packet_t * queue)
{
    packet_t *current = queue;

    fprintf(stderr, "Queue: ");
    while (current != NULL) {
        fprintf(stderr, "%d ", current->id);
        current = (packet_t *) current->next;
    }
    fprintf(stderr, "NULL\n");
}
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

    q_print(Queue);

    fprintf(stderr, "Testing insertions...\n");
    q_insert(&Queue, &four);
    q_print(Queue);
    q_insert(&Queue, &two);
    q_print(Queue);
    q_insert(&Queue, &three);
    q_print(Queue);
    q_insert(&Queue, &eight);
    q_print(Queue);
    q_insert(&Queue, &one);
    q_print(Queue);
    q_insert(&Queue, &one);
    q_print(Queue);

    q_insert(&Queue, &seven);
    q_print(Queue);

    fprintf(stderr, "Testing removes of first element...\n");
    for (i = 0; i < 10; i++) {
        q_extract_first(&Queue);
        q_print(Queue);
    }

    return 0;
}
