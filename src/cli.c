#include "parking.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ParkingLot *init();
int arrive(ParkingLot *p, const char *plate);
int depart(ParkingLot *p, const char *plate);

void print_status(ParkingLot *p) {
  printf("\n--- Parking Lot Status ---\n");
  printf("Filled slots: %d | Waiting: %d\n", p->filled, p->queued.curr);

  printf("Slots:\n");
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1)
      printf("  Slot %d: %s\n", i, p->slots[i].plate);
    else
      printf("  Slot %d: [empty]\n", i);
  }

  printf("Queue:\n");
  if (isEmpty(&p->queued)) {
    printf("  [empty]\n");
  } else {
    int idx = p->queued.front;
    for (int i = 0; i < p->queued.curr; i++) {
      printf("  %s\n", p->queued.arr[idx].plate);
      idx = (idx + 1) % MAX_QUEUE;
    }
  }
  printf("--------------------------\n");
}

int main() {
  ParkingLot *lot = init();

  char plate[16];
  for (int i = 0; i < 21; i++) {
    sprintf(plate, "CAR%02d", i + 1);
    arrive(lot, plate);
  }

  printf("Parking Lot CLI Simulator\n");
  printf("Commands: arrive <plate>, depart <plate>, status, quit\n");

  char command[32], arg[32];
  while (1) {
    printf("> ");
    if (scanf("%s", command) != 1)
      break;

    if (strcmp(command, "arrive") == 0) {
      if (scanf("%s", arg) != 1)
        continue;
      int res = arrive(lot, arg);
      if (res == 0)
        printf("Car %s parked in slot.\n", arg);
      else if (res == 1)
        printf("Car %s added to waiting queue.\n", arg);
      else
        printf("Queue full, car %s cannot enter.\n", arg);
    } else if (strcmp(command, "depart") == 0) {
      if (scanf("%s", arg) != 1)
        continue;
      if (depart(lot, arg) == 0) {
        printf("Car %s departed.\n", arg);
      } else {
        printf("Car %s not found.\n", arg);
      }
    } else if (strcmp(command, "status") == 0) {
      print_status(lot);
    } else if (strcmp(command, "quit") == 0) {
      break;
    } else {
      printf("Unknown command.\n");
    }
  }

  free(lot);
  return 0;
}
