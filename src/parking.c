#include <parking.h>
#include <queue.h>
#include <stdlib.h>
#include <string.h>

ParkingLot *init() {
  ParkingLot *p = (ParkingLot *)malloc(sizeof(ParkingLot));
  p->filled = 0;
  p->waiting = 0;
  p->queued.front = p->queued.rear = -1;
  p->queued.curr = 0;
  for (int i = 0; i < MAX_SLOT; i++) {
    p->slots[i].slot = -1;
  }
  for (int i = 0; i < MAX_QUEUE; i++) {
    p->queued.arr[i].slot = -1;
  }
  return p;
}

int arrive(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot == -1) {
      strcpy(p->slots[i].plate, plate);
      p->slots[i].slot = i;
      p->filled++;
      return 0;
    }
  }
  Car c = {0};
  strcpy(c.plate, plate);
  c.slot = -1;
  if (enqueue(&p->queued, c) == 0) {
    p->waiting = p->queued.curr;
    return 1;
  }
  return 2; // Full queue
}

int depart(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plate) == 0) {
      p->slots[i].slot = -1;
      p->slots[i].plate[0] = '\0';
      p->filled--;

      // Checking wait queue
      if (!isEmpty(&p->queued)) {
        Car c = dequeue(&p->queued);
        if (strcmp(c.plate, "INVALID") != 0) {
          strcpy(p->slots[i].plate, c.plate);
          p->slots[i].slot = i;
          p->filled++;
        }
      }
      p->waiting = p->queued.curr;
      return 0;
    }
  }
  return 1;
}
