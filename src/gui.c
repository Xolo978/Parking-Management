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
static float hoverAnimTime[MAX_SLOT + MAX_QUEUE] = {0};
static int hoveredSlot = -1;

void init_gui(int width, int height) {
  screenW = width;
  screenH = height;
  InitWindow(screenW, screenH, "Parking Lot Management System");
  SetTargetFPS(60);
  srand(time(NULL));
}

void shut() { CloseWindow(); }

static Color lerp_color(Color a, Color b, float t) {
  return (Color){(unsigned char)(a.r + (b.r - a.r) * t),
                 (unsigned char)(a.g + (b.g - a.g) * t),
                 (unsigned char)(a.b + (b.b - a.b) * t),
                 (unsigned char)(a.a + (b.a - a.a) * t)};
}

static void draw_slot(ParkingLot *p, void *data, int count, const char *title,
                      int yStart, bool isParking, bool isQueue) {
  int slotW = 140, slotH = 60;
  int spacing = 12;
  int fontSize = 18;
  int maxPerRow = 5;

  DrawText(title, 20, yStart - fontSize - 30, fontSize + 6,
           (Color){40, 40, 40, 255});
  DrawRectangle(20, yStart - 15, MeasureText(title, fontSize + 6), 3,
                (Color){59, 130, 246, 255});

  int totalRows = (count + maxPerRow - 1) / maxPerRow;

  for (int row = 0; row < totalRows; row++) {
    int slotsInRow =
        (row == totalRows - 1) ? (count - row * maxPerRow) : maxPerRow;
    int rowWidth = slotsInRow * slotW + (slotsInRow - 1) * spacing;
    float startX = (screenW - 300 - rowWidth) / 2.0f;
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

      Color baseColor;
      if (isQueue) {
        baseColor =
            occupied ? (Color){251, 146, 60, 255} : (Color){226, 232, 240, 255};
      } else {
        baseColor =
            occupied ? (Color){239, 68, 68, 255} : (Color){34, 197, 94, 255};
      }

      Rectangle slot = {startX + i * (slotW + spacing), startY, (float)slotW,
                        (float)slotH};

      bool isHovered = CheckCollisionPointRec(GetMousePosition(), slot);
      int globalIdx = isParking ? idx : (MAX_SLOT + idx);

      if (isHovered) {
        hoverAnimTime[globalIdx] += GetFrameTime() * 5.0f;
        if (hoverAnimTime[globalIdx] > 1.0f)
          hoverAnimTime[globalIdx] = 1.0f;
        hoveredSlot = globalIdx;
      } else {
        hoverAnimTime[globalIdx] -= GetFrameTime() * 5.0f;
        if (hoverAnimTime[globalIdx] < 0.0f)
          hoverAnimTime[globalIdx] = 0.0f;
      }

      float lift = hoverAnimTime[globalIdx] * 4.0f;
      Rectangle animSlot = {slot.x, slot.y - lift, slot.width, slot.height};

      DrawRectangle((int)animSlot.x + 2, (int)animSlot.y + 2,
                    (int)animSlot.width, (int)animSlot.height,
                    (Color){0, 0, 0, 40});

      Color slotColor =
          lerp_color(baseColor, WHITE, hoverAnimTime[globalIdx] * 0.15f);
      DrawRectangleRec(animSlot, slotColor);

      Color borderColor =
          isHovered ? (Color){59, 130, 246, 255} : (Color){100, 100, 100, 180};
      DrawRectangleLinesEx(animSlot, 2.0f, borderColor);

      if (occupied) {
        DrawRectangle((int)animSlot.x + 4, (int)animSlot.y + 4,
                      (int)animSlot.width - 8, (int)animSlot.height - 8,
                      (Color){255, 255, 255, 30});
      }

      const char *text = (occupied && car) ? car->plate : "EMPTY";
      int textWidth = MeasureText(text, fontSize);
      int textX = (int)(animSlot.x + (animSlot.width - textWidth) / 2);
      int textY = (int)(animSlot.y + (animSlot.height - fontSize) / 2);

      Color textColor = occupied ? WHITE : (Color){100, 100, 100, 255};
      DrawText(text, textX, textY, fontSize, textColor);

      if (occupied && car && isHovered &&
          IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && isParking) {
        depart(p, car->plate);
      }

      if (!occupied) {
        const char *slotNum = TextFormat("%d", idx + 1);
        int numSize = 12;
        DrawText(slotNum, (int)animSlot.x + 6, (int)animSlot.y + 4, numSize,
                 (Color){150, 150, 150, 150});
      }
    }
  }
}

static void draw_stats(ParkingLot *p) {
  int panelX = screenW - 290;
  int panelY = 60;
  int fontSize = 18;
  int rowHeight = 35;
  int panelWidth = 270;
  int panelHeight = 220;

  DrawRectangle(panelX - 8, panelY - 8, panelWidth, panelHeight,
                (Color){0, 0, 0, 30});

  DrawRectangleGradientV(panelX - 10, panelY - 20, panelWidth, panelHeight,
                         (Color){248, 250, 252, 255},
                         (Color){241, 245, 249, 255});
  DrawRectangleLinesEx(
      (Rectangle){panelX - 10, panelY - 20, panelWidth, panelHeight}, 2.0f,
      (Color){203, 213, 225, 255});

  DrawText("Statistics", panelX + 5, panelY, fontSize + 6,
           (Color){30, 41, 59, 255});
  DrawRectangle(panelX + 5, panelY + 28, 100, 2, (Color){59, 130, 246, 255});

  int startY = panelY + 45;

  DrawRectangle(panelX + 5, startY, 4, 20, (Color){239, 68, 68, 255});
  DrawText(TextFormat("Filled Slots: %d / %d", p->filled, MAX_SLOT),
           panelX + 15, startY, fontSize, (Color){51, 65, 85, 255});

  DrawRectangle(panelX + 5, startY + rowHeight, 4, 20,
                (Color){34, 197, 94, 255});
  DrawText(TextFormat("Free Slots: %d", MAX_SLOT - p->filled), panelX + 15,
           startY + rowHeight, fontSize, (Color){51, 65, 85, 255});

  DrawRectangle(panelX + 5, startY + 2 * rowHeight, 4, 20,
                (Color){251, 146, 60, 255});
  DrawText(TextFormat("Waiting: %d", p->queued.curr), panelX + 15,
           startY + 2 * rowHeight, fontSize, (Color){51, 65, 85, 255});
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
  ClearBackground((Color){249, 250, 251, 255});

  DrawRectangleGradientH(0, 0, screenW, 30, (Color){59, 130, 246, 255},
                         (Color){37, 99, 235, 255});
  DrawText("Parking Lot Management System", 20, 6, 18, WHITE);

  draw_stats(p);

  int spacingY = 100;
  draw_slot(p, p->slots, MAX_SLOT, "Parking Slots", spacingY, true, false);

  int queuedY = spacingY + ((MAX_SLOT + 4) / 5) * (60 + 12) + 50;
  draw_slot(p, &p->queued, MAX_QUEUE, "Waiting Queue", queuedY, false, true);

  handle_hotkeys(p);

  EndDrawing();
}
