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

static void draw_slot(ParkingLot *p, Car *slots, int count, const char *title,
                      int yStart, bool isParking) {
  int slotW = 140, slotH = 60;
  int spacing = 10;
  int fontSize = 20;
  int maxPerRow = 5;

  DrawText(title, 20, yStart - fontSize - 5, fontSize + 4, BLACK);

  int totalRows = (count + maxPerRow - 1) / maxPerRow;

  for (int row = 0; row < totalRows; row++) {
    int slotsInRow =
        (row == totalRows - 1) ? (count - row * maxPerRow) : maxPerRow;
    int rowWidth = slotsInRow * slotW + (slotsInRow - 1) * spacing;
    float startX = (screenW - rowWidth) / 2.0f;
    float startY = yStart + row * (slotH + spacing);

    for (int i = 0; i < slotsInRow; i++) {
      int idx = row * maxPerRow + i;
      if (idx >= count)
        break;

      Rectangle slot = {startX + i * (slotW + spacing), startY, (float)slotW,
                        (float)slotH};

      bool occupied = (slots[idx].slot != -1);
      Color c = occupied ? RED : GREEN;

      if (CheckCollisionPointRec(GetMousePosition(), slot)) {
        DrawRectangleRec(slot, Fade(c, 0.7f));
      } else {
        DrawRectangleRec(slot, c);
      }

      DrawRectangleLines((int)slot.x, (int)slot.y, (int)slot.width,
                         (int)slot.height, BLACK);

      const char *text = occupied ? slots[idx].plate : "EMPTY";
      int textWidth = MeasureText(text, fontSize);
      int textX = (int)(slot.x + (slot.width - textWidth) / 2);
      int textY = (int)(slot.y + (slot.height - fontSize) / 2);

      DrawText(text, textX, textY, fontSize, BLACK);

      if (occupied && CheckCollisionPointRec(GetMousePosition(), slot) &&
          IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {

        if (isParking) {
          depart(p, slots[idx].plate);
        } else {
          slots[idx].slot = -1;
        }
      }
    }
  }
}

void draw(ParkingLot *p) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  int spacingY = 40;

  draw_slot(p, p->slots, MAX_SLOT, "Parking Slots", spacingY, true);

  int queuedYStart = spacingY + ((MAX_SLOT + 4) / 5) * (60 + 10) + 50;

  draw_slot(p, p->queued, MAX_QUEUE, "Queued Cars", queuedYStart, false);

  EndDrawing();
}
