#include "gui.h"
#include "parking.h"
#include "raylib.h"
#include <stdio.h>

int main() {
  ParkingLot *p = init();
  init_gui(1200, 600);
  printf("GUI Init");
  int i = 1;
  char buf[20];
  while (i <= 12) {
    sprintf(buf, "WB-%02d", i);
    arrive(p, buf);
    i++;
  }
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_D))
      depart(p, "WB-01");
    draw(p);
  }
  shut();
  return 0;
}
