#include <parking.h>
#include <stdio.h>

void init(ParkingLot *p) {
  p->filled = 0;
  p->waiting = 0;
}
