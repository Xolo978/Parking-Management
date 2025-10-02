#include "gui.h"
#include "parking.h"
#include "raylib.h"
#include <stdio.h>

int main() {
  ParkingLot *p = init();
  if (p) {
    printf("Filled:%d\n", p->filled);
  }
  init_gui(800, 800);
  printf("GUI Init");
  arrive(p, "WB-01");
  arrive(p, "WB-02");
  while (!WindowShouldClose()) {
    if (IsKeyPressed(KEY_D))
      depart(p, "WB-01");
    draw(p);
  }
  shut();
  return 0;
}
