#include "queue.h"
#include "parking.h"

bool isEmpty(Q *q) {
  if (q->rear == -1 && q->front == -1 && q->curr == 0) {
    return true;
  }
  return false;
}

bool isFull(Q *q) {
  if ((q->rear + 1) % MAX_QUEUE == q->front && q->curr == MAX_QUEUE) {
    return true;
  }
  return false;
}
int enqueue(Q *q, Car car) {
  if (isFull(q)) {
    return 1;
  }
  if (isEmpty(q)) {
    q->front = q->rear = 0;
    q->arr[q->rear] = car;
    q->curr++;
    return 0;
  }
  q->rear = (q->rear + 1) % MAX_QUEUE;
  q->arr[q->rear] = car;
  q->curr++;
  return 0;
}

Car dequeue(Q *q) {
  Car invalid = {.plate = "INVALID", .slot = -2};
  if (isEmpty(q)) {
    return invalid;
  }
  Car car = q->arr[q->front];
  if (q->front == q->rear) {
    q->front = q->rear = -1;
  } else {
    q->front = (q->front + 1) % MAX_QUEUE;
  }
  q->curr--;
  return car;
}
