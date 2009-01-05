#include <stdio.h>
#include <stdint.h>
#include "packet.h"
#include "queue.h"
void printQueue(packet_t * queue)
{
	fprintf(stderr, "printQueue(): ");
	packet_t *current = queue;
	while (current != NULL) {
		printf("%d ", current->id);
		current = current->next;
	}
	printf("NULL\n");
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

	printQueue(Queue);

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

	fprintf(stderr, "Now testing removes\n");
	for (i = 0; i < 10; i++) {
		getFirstInQ(&Queue);
		printQueue(Queue);
	}

	return 0;
}
