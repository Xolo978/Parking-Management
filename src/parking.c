#include "types.h"
#include <parking.h>
#include <queue.h>
#include <stdlib.h>
#include <string.h>

ParkingLot *init() {
  ParkingLot *p = (ParkingLot *)malloc(sizeof(ParkingLot));
  if(!p)return NULL;
  p->filled = 0;
  p->waiting = 0;
  p->queued.front = p->queued.rear = -1;
  p->queued.curr = 0;
  p->revenue = 0.0f;

  for (int i = 0; i < MAX_SLOT; i++) {
    p->slots[i].slot = -1;  
     p->slots[i].arrival = 0;
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
      //Current time
      p->slots[i].arrival = time(NULL);
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
  return 2;
}

static float calculate_duration(time_t arrival){
  time_t now = time(NULL);
  double seconds = difftime(now, arrival);
  return (float)(seconds / 3600.0); 
}

static float calulate_bill(time_t arrival){
  time_t now = time(NULL);
  double seconds = difftime(now, arrival);
  int intervals = (int)(seconds / 30.0);
  if (intervals < 1) intervals = 1;
  return (float)(intervals * 10.0);
}

int depart(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plate) == 0) {

      float bill = calulate_bill(p->slots[i].arrival);
      p->revenue += bill;

      p->slots[i].slot = -1;
      p->slots[i].plate[0] = '\0';
      p->slots[i].arrival = 0;
      p->filled--;

      if (!isEmpty(&p->queued)) {
        Car c = dequeue(&p->queued);
        if (strcmp(c.plate, "INVALID") != 0) {
          strcpy(p->slots[i].plate, c.plate);
          p->slots[i].slot = i;
          p->slots[i].arrival = time(NULL);
          p->filled++;
        }
      }
      p->waiting = p->queued.curr;
      return 0;
    }
  }
  return 1;
}

float park_duration(ParkingLot *p,const char *plate){
  for(int i=0;i<MAX_SLOT;i++){
    if(p->slots[i].slot != -1 && strcmp(p->slots[i].plate,plate)==0){
      return calculate_duration(p->slots[i].arrival);
    }
  }
  return 0.0f;
}

float current_bill(ParkingLot *p,const char *plate){
  for(int i=0;i<MAX_SLOT;i++){
    if(p->slots[i].slot != -1 && strcmp(p->slots[i].plate,plate)==0){
      return calulate_bill(p->slots[i].arrival);
    }
  }
  return 0.0f;
}