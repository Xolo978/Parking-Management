#include "parking.h"
#include <stdbool.h>
#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue {
  Car arr[MAX_QUEUE];
  int rear;
  int front;
  int curr;
} Q;

Q *build_q();
int rear(Q *q);
bool isEmpty(Q *q);
bool isFull(Q *q);
int enqueue(Q *q, Car car);
Car dequeue(Q *q);

#endif
