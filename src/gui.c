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
  int spacing = 10;
  int fontSize = 20;
  int maxPerRow = 10; // max slots per row

  int totalRows = (MAX_SLOT + maxPerRow - 1) / maxPerRow;

  for (int row = 0; row < totalRows; row++) {
    int slotsInRow =
        (row == totalRows - 1) ? (MAX_SLOT - row * maxPerRow) : maxPerRow;

    // Total width of this row
    int rowWidth = slotsInRow * slotW + (slotsInRow - 1) * spacing;

    // Start X to horizontally center the row
    float startX = (screenW - rowWidth) / 2.0f;
    float startY = 40 + row * (slotH + spacing);

    for (int i = 0; i < slotsInRow; i++) {
      int idx = row * maxPerRow + i; // actual slot index

      Rectangle slot = {startX + i * (slotW + spacing), startY, (float)slotW,
                        (float)slotH};

      bool occupied = (p->slots[idx].slot != -1);
      Color c = occupied ? RED : GREEN;
      DrawRectangleRec(slot, c);
      DrawRectangleLines((int)slot.x, (int)slot.y, (int)slot.width,
                         (int)slot.height, BLACK);

      const char *text = occupied ? p->slots[idx].plate : "EMPTY";
      int textWidth = MeasureText(text, fontSize);

      int textX = (int)(slot.x + (slot.width - textWidth) / 2);
      int textY = (int)(slot.y + (slot.height - fontSize) / 2);

      DrawText(text, textX, textY, fontSize, BLACK);
    }
  }

  EndDrawing();
}
