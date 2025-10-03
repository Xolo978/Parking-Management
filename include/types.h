#ifndef TYPES_H
#define TYPES_H

#define MAX_SLOT 20
#define MAX_QUEUE 10

typedef struct {
  char plate[32];
  int slot;
} Car;

typedef struct Queue {
  Car arr[MAX_QUEUE];
  int rear;
  int front;
  int curr;
} Q;

typedef struct {
  int filled;
  int waiting;
  Car slots[MAX_SLOT];
  Q queued;
} ParkingLot;

#endif
