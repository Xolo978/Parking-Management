#include "gui.h"
#include "parking.h"
#include "raylib.h"

static int screenW, screenH;

void init_gui(int width, int height) {
  screenH = height;
  screenW = width;
  InitWindow(screenW, screenH, "Parking Lot");
  SetTargetFPS(60);
}

void shut() { CloseWindow(); }

void draw(ParkingLot *p) {
  BeginDrawing();
  ClearBackground(RAYWHITE);
  int slotW = 140, slotH = 60;
  int startX = 40, startY = 40;
  for (int i = 0; i < MAX_SLOT; i++) {
    Rectangle slot = {(float)(startX + i * (slotW + 10)), (float)startY,
                      (float)slotW, (float)slotH};
    bool occupied = (p->slots[i].slot != -1);
    Color c = occupied ? RED : GREEN;
    DrawRectangleRec(slot, c);
    DrawRectangleLines((int)slot.x, (int)slot.y, (int)slot.width,
                       (int)slot.height, BLACK);
    if (occupied) {
      DrawText(p->slots[i].plate, (int)slot.x + 8, (int)slot.y + 20, 20, BLACK);
    } else {
      DrawText("EMPTY", (int)slot.x + 8, (int)slot.y + 20, 20, BLACK);
    }
  }
  EndDrawing();
}
