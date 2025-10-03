#ifndef QUEUE_H
#define QUEUE_H

#include "types.h"
#include <stdbool.h>

Q *build_q();
int rear(Q *q);
bool isEmpty(Q *q);
bool isFull(Q *q);
int enqueue(Q *q, Car car);
Car dequeue(Q *q);

#endif
