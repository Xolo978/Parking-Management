#include "gui.h"
#include "parking.h"
#include "queue.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <types.h>

static int screenW, screenH;

void init_gui(int width, int height) {
  screenW = width;
  screenH = height;
  InitWindow(screenW, screenH, "Parking Lot");
  SetTargetFPS(60);
  srand(time(NULL));
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

      Color c =
          isQueue ? (occupied ? ORANGE : LIGHTGRAY) : (occupied ? RED : GREEN);

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

      if (occupied && car && CheckCollisionPointRec(GetMousePosition(), slot) &&
          IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && isParking) {
        depart(p, car->plate);
      }
    }
  }
}

static void draw_stats(ParkingLot *p) {
  int panelX = 20;
  int panelY = 20;
  int fontSize = 20;

  DrawText("=== Parking Stats ===", panelX, panelY, fontSize + 4, DARKGRAY);
  DrawText(TextFormat("Filled Slots : %d", p->filled), panelX, panelY + 30,
           fontSize, BLACK);
  DrawText(TextFormat("Free Slots   : %d", MAX_SLOT - p->filled), panelX,
           panelY + 60, fontSize, BLACK);
  DrawText(TextFormat("Waiting Cars : %d", p->queued.curr), panelX, panelY + 90,
           fontSize, BLACK);
}

static void handle_hotkeys(ParkingLot *p) {
  static int counter = 100;

  if (IsKeyPressed(KEY_A)) {
    char plate[16];
    sprintf(plate, "CAR%03d", counter++);
    arrive(p, plate);
  }

  if (IsKeyPressed(KEY_D)) {
    for (int i = 0; i < MAX_SLOT; i++) {
      if (p->slots[i].slot != -1) {
        depart(p, p->slots[i].plate);
        break;
      }
    }
  }

  if (IsKeyPressed(KEY_Q)) {
    if (!isEmpty(&p->queued)) {
      for (int i = 0; i < MAX_SLOT; i++) {
        if (p->slots[i].slot == -1) {
          Car c = dequeue(&p->queued);
          if (strcmp(c.plate, "INVALID") != 0) {
            strcpy(p->slots[i].plate, c.plate);
            p->slots[i].slot = i;
            p->filled++;
          }
          break;
        }
      }
    }
  }
}

void draw(ParkingLot *p) {
  BeginDrawing();
  ClearBackground(RAYWHITE);

  draw_stats(p);

  int spacingY = 150;
  draw_slot(p, p->slots, MAX_SLOT, "Parking Slots", spacingY, true, false);

  int queuedYStart = spacingY + ((MAX_SLOT + 4) / 5) * (60 + 10) + 50;
  draw_slot(p, &p->queued, MAX_QUEUE, "Queued Cars", queuedYStart, false, true);

  handle_hotkeys(p);

  EndDrawing();
}
