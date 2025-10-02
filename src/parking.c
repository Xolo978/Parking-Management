#include <parking.h>
#include <stdlib.h>
#include <string.h>

ParkingLot *init() {
  ParkingLot *p = (ParkingLot *)malloc(sizeof(ParkingLot));
  p->filled = 0;
  p->waiting = 0;
  for (int i = 0; i < MAX_SLOT; i++) {
    p->slots[i].slot = -1;
  }
  for (int i = 0; i < MAX_QUEUE; i++) {
    p->queued[i].slot = -1;
  }
  return p;
}

void arrive(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot == -1) {
      strcpy(p->slots[i].plate, plate);
      p->slots[i].slot = i;
      p->filled++;
      return;
    }
  }
  strcpy(p->queued[p->waiting].plate, plate);
  p->queued[p->waiting].slot = p->waiting;
  p->waiting++;
}

void depart(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plate) == 0) {
      p->slots[i].slot = -1;
      if (p->waiting > 0) {
        strcpy(p->slots[i].plate, p->queued[0].plate);
        p->slots[i].slot = i;
        p->queued[i].slot = -1;
        p->waiting--;
        return;
      }
    }
  }
  for (int i = 0; i < p->waiting; i++) {
    if (strcmp(p->queued[i].plate, plate) == 0) {
      p->queued[i].slot = -1;
      p->waiting--;
      return;
    }
  }
}
