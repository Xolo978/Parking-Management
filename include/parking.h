#ifndef PARKING_H
#define PARKING_H

#define MAX_SLOT 10
#define MAX_QUEUE 20

typedef struct {
  char plate[10];
  int slot;
} Car;

typedef struct {
  int filled;
  int waiting;
  Car slots[MAX_SLOT];
  Car queued[MAX_QUEUE];
} ParkingLot;

void arrive(ParkingLot *p, const char *plate);
void depart(ParkingLot *p, const char *plate);
ParkingLot status(ParkingLot *p);
ParkingLot *init();

#endif
