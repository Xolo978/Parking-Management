#include "gui.h"
#include "parking.h"
#include "queue.h"
#include "raylib.h"
#include "types.h"
#include <string.h>

static int screenW, screenH;

void init_gui(int width, int height) {
  screenH = height;
  screenW = width;
  InitWindow(screenW, screenH, "Parking Lot");
  SetTargetFPS(60);
}

void shut() { CloseWindow(); }

static void draw_slot(ParkingLot *p, void *data, int count, const char *title,
                      int yStart, bool isParking, bool isQueue) {
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

      Car *car = NULL;
      bool occupied = false;

      if (isQueue) {
        Q *q = (Q *)data;
        if (idx < q->curr) {
          int pos = (q->front + idx) % MAX_QUEUE;
          car = &q->arr[pos];
          occupied = (car->plate[0] != '\0');
        }
      } else {
        Car *arr = (Car *)data;
        car = &arr[idx];
        occupied = (car->slot != -1);
      }

      Color c;
      if (isQueue) {
        c = occupied ? ORANGE : LIGHTGRAY;
      } else {
        c = occupied ? RED : GREEN;
      }

      Rectangle slot = {startX + i * (slotW + spacing), startY, (float)slotW,
                        (float)slotH};

      if (CheckCollisionPointRec(GetMousePosition(), slot)) {
        DrawRectangleRec(slot, Fade(c, 0.7f));
      } else {
        DrawRectangleRec(slot, c);
      }

      DrawRectangleLines((int)slot.x, (int)slot.y, (int)slot.width,
                         (int)slot.height, BLACK);

      const char *text = (occupied && car) ? car->plate : "EMPTY";
      int textWidth = MeasureText(text, fontSize);
      int textX = (int)(slot.x + (slot.width - textWidth) / 2);
      int textY = (int)(slot.y + (slot.height - fontSize) / 2);

      DrawText(text, textX, textY, fontSize, BLACK);

      // Right-click removal
      if (occupied && car && CheckCollisionPointRec(GetMousePosition(), slot) &&
          IsMouseButtonPressed(MOUSE_RIGHT_BUTTON)) {
        if (isParking) {
          depart(p, car->plate);
        } else if (isQueue) {
          dequeue(&p->queued);
        }
      }
    }
  }
}

void draw(ParkingLot *p) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  int spacingY = 40;

  draw_slot(p, p->slots, MAX_SLOT, "Parking Slots", spacingY, true, false);

  int queuedYStart = spacingY + ((MAX_SLOT + 4) / 5) * (60 + 10) + 50;
  draw_slot(p, &p->queued, MAX_QUEUE, "Queued Cars", queuedYStart, false, true);

  EndDrawing();
}
