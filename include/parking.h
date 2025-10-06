#ifndef PARKING_H
#define PARKING_H

#include "types.h"

int arrive(ParkingLot *p, const char *plate);
int depart(ParkingLot *p, const char *plate);
float park_duration(ParkingLot *p, const char *plate);
float current_bill(ParkingLot *p, const char *plate);
ParkingLot status(ParkingLot *p);
ParkingLot *init();

#endif
