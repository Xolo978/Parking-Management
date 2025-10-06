#include "gui.h"
#include "parking.h"
#include "queue.h"
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <types.h>

static int screenW, screenH;
static float hoverAnimTime[MAX_SLOT + MAX_QUEUE] = {0};
static int hoveredSlot = -1;
static Font customFont;
static bool fontLoaded = false;

static char searchQuery[32] = "";
static int searchResultSlot = -1;
static bool searchActive = false;

static char plateInput[32] = "";
static bool inputActive = false;

static int autoCounter = 100;

static char currentTooltip[128] = "";
static bool showTooltip = false;
static int tooltipX = 0;
static int tooltipY = 0;

typedef enum {
  ANIM_NONE,
  ANIM_ARRIVE,
  ANIM_DEPART,
  ANIM_PROMOTE
} AnimationType;

typedef struct {
  AnimationType type;
  int slotIndex;
  float progress;
  char plate[32];
  bool active;
} SlotAnimation;

static SlotAnimation slotAnims[MAX_SLOT + MAX_QUEUE] = {0};

typedef struct {
  char message[64];
  float lifetime;
  Color color;
  bool active;
} Notification;

#define MAX_NOTIFICATIONS 5
static Notification notifications[MAX_NOTIFICATIONS] = {0};

static void trigger_animation(AnimationType type, int slotIndex, const char *plate);

static void add_notification(const char *message, Color color) {
  for (int i = MAX_NOTIFICATIONS - 1; i > 0; i--) {
    notifications[i] = notifications[i - 1];
  }
  notifications[0].active = true;
  notifications[0].lifetime = 3.0f;
  notifications[0].color = color;
  strncpy(notifications[0].message, message, 63);
  notifications[0].message[63] = '\0';
}

static void update_notifications() {
  for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
    if (notifications[i].active) {
      notifications[i].lifetime -= GetFrameTime();
      if (notifications[i].lifetime <= 0.0f) {
        notifications[i].active = false;
      }
    }
  }
}

static void draw_notifications() {
  int startY = 380;
  int spacing = 35;
  int panelX = screenW - 290;
  
  for (int i = 0; i < MAX_NOTIFICATIONS; i++) {
    if (notifications[i].active) {
      float alpha = notifications[i].lifetime > 0.5f ? 1.0f : (notifications[i].lifetime / 0.5f);
      int yPos = startY + i * spacing;
      
      int textWidth = MeasureText(notifications[i].message, 18);
      int panelW = textWidth + 20;
      
      DrawRectangle(panelX + 2, yPos + 2, panelW, 28, 
                   (Color){0, 0, 0, (unsigned char)(60 * alpha)});
      
      Color bgColor = notifications[i].color;
      bgColor.a = (unsigned char)(220 * alpha);
      DrawRectangle(panelX, yPos, panelW, 28, bgColor);
      DrawRectangleLinesEx((Rectangle){panelX, yPos, panelW, 28}, 2.0f,
                          (Color){255, 255, 255, (unsigned char)(180 * alpha)});
      
      DrawText(notifications[i].message, panelX + 10, yPos + 7, 18,
              (Color){255, 255, 255, (unsigned char)(255 * alpha)});
    }
  }
}

static bool is_duplicate(ParkingLot *p, const char *plate) {
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1) {
      if (strcmp(p->slots[i].plate, plate) == 0) {
        return true;
      }
    }
  }
  
  Q *q = &p->queued;
  int count = (q->rear - q->front + MAX_QUEUE) % MAX_QUEUE;
  if (q->curr > 0) count = q->curr;
  
  for (int i = 0; i < count; i++) {
    int idx = (q->front + i) % MAX_QUEUE;
    if (strcmp(q->arr[idx].plate, plate) == 0) {
      return true;
    }
  }
  
  return false;
}

static void search_car(ParkingLot *p, const char *query) {
  searchResultSlot = -1;
  
  if (strlen(query) == 0) {
    return;
  }
  
  for (int i = 0; i < MAX_SLOT; i++) {
    if (p->slots[i].slot != -1) {
      if (strstr(p->slots[i].plate, query) != NULL) {
        searchResultSlot = i;
        return;
      }
    }
  }
  
  for (int i = 0; i < p->queued.curr; i++) {
    int pos = (p->queued.front + i) % MAX_QUEUE;
    if (strstr(p->queued.arr[pos].plate, query) != NULL) {
      searchResultSlot = MAX_SLOT + i;
      return;
    }
  }
}

static void draw_search_box() {
  int boxX = 30;
  int boxY = 55;
  int boxW = 250;
  int boxH = 35;
  
  Rectangle searchBox = {boxX, boxY, boxW, boxH};
  bool isHovered = CheckCollisionPointRec(GetMousePosition(), searchBox);
  
  DrawRectangle(boxX + 2, boxY + 2, boxW, boxH, (Color){0, 0, 0, 40});
  
  Color bgColor = searchActive ? (Color){255, 255, 255, 255} : (Color){248, 250, 252, 255};
  DrawRectangle(boxX, boxY, boxW, boxH, bgColor);
  
  Color borderColor = searchActive ? (Color){59, 130, 246, 255} : 
                      (isHovered ? (Color){148, 163, 184, 255} : (Color){203, 213, 225, 255});
  DrawRectangleLinesEx(searchBox, 2.0f, borderColor);
  
  const char *placeholder = "Search license plate...";
  if (strlen(searchQuery) == 0 && !searchActive) {
    DrawText(placeholder, boxX + 10, boxY + 10, 20, (Color){148, 163, 184, 255});
  } else {
    DrawText(searchQuery, boxX + 10, boxY + 10, 20, (Color){30, 41, 59, 255});
    
    if (searchActive) {
      int textWidth = MeasureText(searchQuery, 20);
      int cursorX = boxX + 10 + textWidth;
      if ((int)(GetTime() * 2) % 2 == 0) {
        DrawRectangle(cursorX, boxY + 8, 2, 20, (Color){59, 130, 246, 255});
      }
    }
  }
  
  DrawText("🔍", boxX + boxW - 30, boxY + 8, 20, (Color){100, 116, 139, 255});
  
  if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    searchActive = true;
    inputActive = false;
  }
}

static void draw_input_box() {
  int boxX = 300;
  int boxY = 55;
  int boxW = 220;
  int boxH = 35;
  
  Rectangle inputBox = {boxX, boxY, boxW, boxH};
  bool isHovered = CheckCollisionPointRec(GetMousePosition(), inputBox);
  
  DrawRectangle(boxX + 2, boxY + 2, boxW, boxH, (Color){0, 0, 0, 40});
  
  Color bgColor = inputActive ? (Color){255, 255, 255, 255} : (Color){248, 250, 252, 255};
  DrawRectangle(boxX, boxY, boxW, boxH, bgColor);
  
  Color borderColor = inputActive ? (Color){34, 197, 94, 255} : 
                      (isHovered ? (Color){148, 163, 184, 255} : (Color){203, 213, 225, 255});
  DrawRectangleLinesEx(inputBox, 2.0f, borderColor);
  
  const char *placeholder = "Enter plate number...";
  if (strlen(plateInput) == 0 && !inputActive) {
    DrawText(placeholder, boxX + 10, boxY + 10, 20, (Color){148, 163, 184, 255});
  } else {
    DrawText(plateInput, boxX + 10, boxY + 10, 20, (Color){30, 41, 59, 255});
    
    if (inputActive) {
      int textWidth = MeasureText(plateInput, 20);
      int cursorX = boxX + 10 + textWidth;
      if ((int)(GetTime() * 2) % 2 == 0) {
        DrawRectangle(cursorX, boxY + 8, 2, 20, (Color){34, 197, 94, 255});
      }
    }
  }
  
  if (isHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    inputActive = true;
    searchActive = false;
  }
}

static void draw_add_buttons(ParkingLot *p) {
  int btnY = 55;
  int btnH = 35;
  int spacing = 10;
  
  int addX = 530;
  int addW = 120;
  Rectangle addBtn = {addX, btnY, addW, btnH};
  bool addHovered = CheckCollisionPointRec(GetMousePosition(), addBtn);
  
  DrawRectangle(addX + 2, btnY + 2, addW, btnH, (Color){0, 0, 0, 40});
  
  Color addBg = addHovered ? (Color){22, 163, 74, 255} : (Color){34, 197, 94, 255};
  DrawRectangle(addX, btnY, addW, btnH, addBg);
  DrawRectangleLinesEx(addBtn, 2.0f, (Color){255, 255, 255, 200});
  
  const char *addText = "Add Car";
  int textW = MeasureText(addText, 20);
  DrawText(addText, addX + (addW - textW) / 2, btnY + 10, 20, WHITE);
  
  if (addHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    if (strlen(plateInput) > 0) {
      if (is_duplicate(p, plateInput)) {
        add_notification("Car already exists!", (Color){239, 68, 68, 255});
      } else {
        int result = arrive(p, plateInput);
        
        if (result == 0) {
          for (int i = 0; i < MAX_SLOT; i++) {
            if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plateInput) == 0) {
              trigger_animation(ANIM_ARRIVE, i, plateInput);
              break;
            }
          }
        } else if (result == 1) {
          int qIdx = p->queued.curr - 1;
          if (qIdx >= 0) {
            trigger_animation(ANIM_ARRIVE, MAX_SLOT + qIdx, plateInput);
          }
        }
      }
      
      plateInput[0] = '\0';
    }
  }
  
  int randomX = addX + addW + spacing;
  int randomW = 140;
  Rectangle randomBtn = {randomX, btnY, randomW, btnH};
  bool randomHovered = CheckCollisionPointRec(GetMousePosition(), randomBtn);
  
  DrawRectangle(randomX + 2, btnY + 2, randomW, btnH, (Color){0, 0, 0, 40});
  
  Color randomBg = randomHovered ? (Color){29, 78, 216, 255} : (Color){59, 130, 246, 255};
  DrawRectangle(randomX, btnY, randomW, btnH, randomBg);
  DrawRectangleLinesEx(randomBtn, 2.0f, (Color){255, 255, 255, 200});
  
  const char *randomText = "Add Random";
  int randomTextW = MeasureText(randomText, 20);
  DrawText(randomText, randomX + (randomW - randomTextW) / 2, btnY + 10, 20, WHITE);
  
  if (randomHovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    char plate[16];
    sprintf(plate, "CAR%03d", autoCounter++);
    
    if (is_duplicate(p, plate)) {
      add_notification("Car already exists!", (Color){239, 68, 68, 255});
    } else {
      int result = arrive(p, plate);
      
      if (result == 0) {
        for (int i = 0; i < MAX_SLOT; i++) {
          if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plate) == 0) {
            trigger_animation(ANIM_ARRIVE, i, plate);
            break;
          }
        }
      } else if (result == 1) {
        int qIdx = p->queued.curr - 1;
        if (qIdx >= 0) {
          trigger_animation(ANIM_ARRIVE, MAX_SLOT + qIdx, plate);
        }
      }
    }
  }
}

static void trigger_animation(AnimationType type, int slotIndex, const char *plate) {
  slotAnims[slotIndex].type = type;
  slotAnims[slotIndex].slotIndex = slotIndex;
  slotAnims[slotIndex].progress = 0.0f;
  slotAnims[slotIndex].active = true;
  if (plate) {
    strncpy(slotAnims[slotIndex].plate, plate, 31);
    slotAnims[slotIndex].plate[31] = '\0';
  }
  
  if (type == ANIM_ARRIVE) {
    char msg[64];
    snprintf(msg, 64, "%s arrived", plate);
    add_notification(msg, (Color){34, 197, 94, 255});
  } else if (type == ANIM_DEPART) {
    char msg[64];
    snprintf(msg, 64, "%s departed", plate);
    add_notification(msg, (Color){239, 68, 68, 255});
  } else if (type == ANIM_PROMOTE) {
    char msg[64];
    snprintf(msg, 64, "%s promoted from queue", plate);
    add_notification(msg, (Color){59, 130, 246, 255});
  }
}

static void update_animations() {
  for (int i = 0; i < MAX_SLOT + MAX_QUEUE; i++) {
    if (slotAnims[i].active) {
      slotAnims[i].progress += GetFrameTime() * 2.0f;
      if (slotAnims[i].progress >= 1.0f) {
        slotAnims[i].active = false;
        slotAnims[i].progress = 0.0f;
      }
    }
  }
}

void init_gui(int width, int height) {
  screenW = width;
  screenH = height;
  InitWindow(screenW, screenH, "Parking Lot Management System");
  SetTargetFPS(60);
  srand(time(NULL));
}

void shut() { 
  if (fontLoaded && customFont.texture.id != GetFontDefault().texture.id) {
    UnloadFont(customFont);
  }
  CloseWindow(); 
}

static Color lerp_color(Color a, Color b, float t) {
  return (Color){(unsigned char)(a.r + (b.r - a.r) * t),
                 (unsigned char)(a.g + (b.g - a.g) * t),
                 (unsigned char)(a.b + (b.b - a.b) * t),
                 (unsigned char)(a.a + (b.a - a.a) * t)};
}

static void draw_tooltip(const char *text, int x, int y) {
  int fontSize = 18;
  int padding = 8;
  int textWidth = MeasureText(text, fontSize);
  int tooltipWidth = textWidth + padding * 2;
  int tooltipHeight = fontSize + padding * 2;
  
  DrawRectangle(x + 3, y + 3, tooltipWidth, tooltipHeight, (Color){0, 0, 0, 80});
  
  DrawRectangleGradientV(x, y, tooltipWidth, tooltipHeight, 
                         (Color){45, 55, 72, 240}, (Color){26, 32, 44, 240});
  DrawRectangleLinesEx((Rectangle){x, y, tooltipWidth, tooltipHeight}, 1.5f, 
                       (Color){99, 179, 237, 200});
  
  
  DrawText(text, x + padding, y + padding, fontSize, (Color){226, 232, 240, 255});
}

static void draw_slot(ParkingLot *p, void *data, int count, const char *title,
                      int yStart, bool isParking, bool isQueue) {
  int slotW = 140, slotH = 70;
  int spacing = 14;
  int fontSize = 22;
  int maxPerRow = 5;

  DrawText(title, 30, yStart - fontSize - 38, fontSize + 8,
           (Color){30, 41, 59, 255});
  DrawRectangleRounded((Rectangle){28, yStart - 18, MeasureText(title, fontSize + 8) + 4, 4}, 
                       0.5f, 8, (Color){59, 130, 246, 255});
  
  DrawRectangle(25, yStart - 25, screenW - 350, 
                ((count + maxPerRow - 1) / maxPerRow) * (slotH + spacing) + 20,
                (Color){0, 0, 0, 15});

  int totalRows = (count + maxPerRow - 1) / maxPerRow;

  for (int row = 0; row < totalRows; row++) {
    int slotsInRow = (row == totalRows - 1) ? (count - row * maxPerRow) : maxPerRow;
    int rowWidth = slotsInRow * slotW + (slotsInRow - 1) * spacing;
    float startX = (screenW - 320 - rowWidth) / 2.0f;
    float startY = yStart + row * (slotH + spacing);

    for (int i = 0; i < slotsInRow; i++) {
      int idx = row * maxPerRow + i;
      if (idx >= count) break;

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
        baseColor = occupied ? (Color){251, 146, 60, 255} : (Color){241, 245, 249, 255};
      } else {
        baseColor = occupied ? (Color){239, 68, 68, 255} : (Color){148, 163, 184, 255};
      }

      Rectangle slot = {startX + i * (slotW + spacing), startY, (float)slotW, (float)slotH};
      bool isHovered = CheckCollisionPointRec(GetMousePosition(), slot);
      int globalIdx = isParking ? idx : (MAX_SLOT + idx);
      
      bool isSearchResult = (searchResultSlot == globalIdx && strlen(searchQuery) > 0);

      if (isHovered) {
        hoverAnimTime[globalIdx] += GetFrameTime() * 6.0f;
        if (hoverAnimTime[globalIdx] > 1.0f) hoverAnimTime[globalIdx] = 1.0f;
        hoveredSlot = globalIdx;
      } else {
        hoverAnimTime[globalIdx] -= GetFrameTime() * 6.0f;
        if (hoverAnimTime[globalIdx] < 0.0f) hoverAnimTime[globalIdx] = 0.0f;
      }

      float lift = hoverAnimTime[globalIdx] * 6.0f;
      float scale = 1.0f + hoverAnimTime[globalIdx] * 0.03f;
      Rectangle animSlot = {
        slot.x - (slotW * scale - slotW) / 2, 
        slot.y - lift - (slotH * scale - slotH) / 2, 
        slotW * scale, 
        slotH * scale
      };

      DrawRectangleRounded(
        (Rectangle){animSlot.x + 4, animSlot.y + 4, animSlot.width, animSlot.height},
        0.15f, 12, (Color){0, 0, 0, (unsigned char)(50 + hoverAnimTime[globalIdx] * 30)}
      );

      Color topColor = lerp_color(baseColor, WHITE, hoverAnimTime[globalIdx] * 0.2f);
      Color bottomColor = lerp_color(baseColor, (Color){0, 0, 0, 30}, hoverAnimTime[globalIdx] * 0.1f);
      
      DrawRectangleGradientV((int)animSlot.x, (int)animSlot.y, 
                             (int)animSlot.width, (int)animSlot.height,
                             topColor, bottomColor);
      
      Color borderColor = isHovered ? (Color){59, 130, 246, 255} : (Color){148, 163, 184, 180};
      DrawRectangleLinesEx(animSlot, 2.5f, borderColor);
      
      if (isSearchResult) {
        float pulse = (sinf((float)GetTime() * 5.0f) + 1.0f) / 2.0f;
        unsigned char alpha = (unsigned char)(100 + pulse * 100);
        DrawRectangleLinesEx(animSlot, 4.0f, (Color){255, 215, 0, alpha});
        DrawRectangleLinesEx((Rectangle){animSlot.x - 3, animSlot.y - 3, animSlot.width + 6, animSlot.height + 6}, 
                            2.0f, (Color){255, 215, 0, (unsigned char)(alpha * 0.5f)});
      }

      if (occupied) {
        DrawRectangleRounded(
          (Rectangle){animSlot.x + 5, animSlot.y + 5, animSlot.width - 10, animSlot.height - 10},
          0.2f, 8, (Color){255, 255, 255, 25}
        );
      }

      if (occupied && isParking) {
        DrawRectangle((int)animSlot.x + 12, (int)animSlot.y + 12, 20, 12, 
                      (Color){255, 255, 255, 180});
        DrawCircle((int)animSlot.x + 18, (int)animSlot.y + 26, 3, 
                   (Color){255, 255, 255, 180});
        DrawCircle((int)animSlot.x + 26, (int)animSlot.y + 26, 3, 
                   (Color){255, 255, 255, 180});
      }

      const char *text = (occupied && car) ? car->plate : "EMPTY";
      int textWidth = MeasureText(text, fontSize);
      int textX = (int)(animSlot.x + (animSlot.width - textWidth) / 2);
      int textY = (int)(animSlot.y + (animSlot.height - fontSize) / 2) + 
                  (occupied && isParking ? 8 : 0);

      Color textColor = occupied ? WHITE : (Color){71, 85, 105, 255};
      DrawText(text, textX, textY, fontSize, textColor);

      if (slotAnims[globalIdx].active) {
        float prog = slotAnims[globalIdx].progress;
        
        if (slotAnims[globalIdx].type == ANIM_ARRIVE) {
          float pulseSize = 30.0f * (1.0f - prog);
          unsigned char alpha = (unsigned char)(200 * (1.0f - prog));
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize, (Color){34, 197, 94, alpha});
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize - 5, (Color){34, 197, 94, (unsigned char)(alpha * 0.5f)});
          
          const char *label = "ARRIVED";
          int labelW = MeasureText(label, 10);
          DrawText(label, (int)(animSlot.x + (animSlot.width - labelW) / 2), 
                  (int)(animSlot.y - 20), 10, (Color){34, 197, 94, alpha});
        }
        else if (slotAnims[globalIdx].type == ANIM_DEPART) {
          float pulseSize = 40.0f * (1.0f - prog);
          unsigned char alpha = (unsigned char)(200 * (1.0f - prog));
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize, (Color){239, 68, 68, alpha});
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize - 5, (Color){239, 68, 68, (unsigned char)(alpha * 0.5f)});
          
          const char *label = "DEPARTED";
          int labelW = MeasureText(label, 10);
          DrawText(label, (int)(animSlot.x + (animSlot.width - labelW) / 2), 
                  (int)(animSlot.y - 20), 10, (Color){239, 68, 68, alpha});
        }
        else if (slotAnims[globalIdx].type == ANIM_PROMOTE) {
          float pulseSize = 35.0f * (1.0f - prog);
          unsigned char alpha = (unsigned char)(200 * (1.0f - prog));
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize, (Color){59, 130, 246, alpha});
          DrawCircleLines((int)(animSlot.x + animSlot.width / 2), 
                         (int)(animSlot.y + animSlot.height / 2), 
                         pulseSize - 5, (Color){59, 130, 246, (unsigned char)(alpha * 0.5f)});
          
          const char *label = "PROMOTED";
          int labelW = MeasureText(label, 10);
          DrawText(label, (int)(animSlot.x + (animSlot.width - labelW) / 2), 
                  (int)(animSlot.y - 20), 10, (Color){59, 130, 246, alpha});
        }
      }

      if (!occupied) {
        const char *slotNum = TextFormat("%d", idx + 1);
        float badgeSize = 22.0f;
        DrawCircle((int)animSlot.x + (int)(badgeSize/2) + 6, (int)animSlot.y + (int)(badgeSize/2) + 6, 
                   badgeSize/2, (Color){100, 116, 139, 120});
        DrawText(slotNum, (int)animSlot.x + 8, (int)animSlot.y + 8, 18,
                 (Color){241, 245, 249, 200});
      }

      // Parking duration for occupied parking slots
      if (occupied && car && isParking) {
        float duration = park_duration(p, car->plate);
        const char *durationText = TextFormat("%.1fh", duration);
        int durWidth = MeasureText(durationText, 12);
        DrawText(durationText, (int)animSlot.x + (int)animSlot.width - durWidth - 8,
                 (int)animSlot.y + 8, 12, (Color){255, 255, 255, 200});
      }


      if (isHovered && occupied && car) {
        showTooltip = true;
        tooltipX = GetMouseX() + 15;
        tooltipY = GetMouseY() + 15;
        if (isParking) {
          float bill = current_bill(p, car->plate);
          snprintf(currentTooltip, sizeof(currentTooltip), "%s | Bill: $%.2f", car->plate, bill);
        } else {
          snprintf(currentTooltip, sizeof(currentTooltip), "%s (Waiting)", car->plate);
        }
      }

      if (occupied && car && isHovered && 
          IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && isParking) {
        char departPlate[32];
        strncpy(departPlate, car->plate, 31);
        departPlate[31] = '\0';
        trigger_animation(ANIM_DEPART, idx, departPlate);
        depart(p, departPlate);
        
        if (p->slots[idx].slot != -1) {
          trigger_animation(ANIM_PROMOTE, idx, p->slots[idx].plate);
        }
      }

    }
  }
}

static void draw_stats(ParkingLot *p) {
  int panelX = screenW - 300;
  int panelY = 70;
  int fontSize = 22;
  int rowHeight = 40;
  int panelWidth = 280;
  int panelHeight = 300;

 
  DrawRectangle(panelX - 5, panelY - 5, panelWidth, panelHeight,
                (Color){0, 0, 0, 40});

  DrawRectangleGradientV(panelX - 8, panelY - 25, panelWidth, panelHeight,
                         (Color){255, 255, 255, 255},
                         (Color){248, 250, 252, 255});
  

  DrawRectangleLinesEx(
      (Rectangle){panelX - 8, panelY - 25, panelWidth, panelHeight}, 
      2.5f, (Color){203, 213, 225, 255});

  DrawText("Statistics", panelX + 8, panelY - 10, fontSize + 8,
           (Color){15, 23, 42, 255});
  DrawRectangle(panelX + 8, panelY + 20, panelWidth - 16, 3, (Color){59, 130, 246, 255});

  int startY = panelY + 38;

  DrawRectangle(panelX + 8, startY, 5, 24, (Color){239, 68, 68, 255});
  DrawText(TextFormat("Filled Slots: %d / %d", p->filled, MAX_SLOT),
           panelX + 20, startY + 3, fontSize, (Color){51, 65, 85, 255});
  
  float fillPercent = (float)p->filled / MAX_SLOT;
  int barWidth = panelWidth - 30;
  DrawRectangle(panelX + 10, startY + 28, barWidth, 8, (Color){226, 232, 240, 255});
  DrawRectangle(panelX + 10, startY + 28, (int)(barWidth * fillPercent), 8, (Color){239, 68, 68, 255});

  DrawRectangle(panelX + 8, startY + rowHeight + 8, 5, 24, (Color){34, 197, 94, 255});
  DrawText(TextFormat("Free Slots: %d", MAX_SLOT - p->filled), panelX + 20,
           startY + rowHeight + 11, fontSize, (Color){51, 65, 85, 255});

  DrawRectangle(panelX + 8, startY + 2 * rowHeight + 16, 5, 24, (Color){251, 146, 60, 255});
  DrawText(TextFormat("Waiting: %d / %d", p->queued.curr, MAX_QUEUE), panelX + 20,
           startY + 2 * rowHeight + 19, fontSize, (Color){51, 65, 85, 255});

  DrawRectangle(panelX + 8, startY + 3 * rowHeight + 24, 5, 24, (Color){168, 85, 247, 255});
  DrawText(TextFormat("Revenue: $%.2f", p->revenue), panelX + 20,
           startY + 3 * rowHeight + 27, fontSize, (Color){51, 65, 85, 255});

}

static void handle_hotkeys(ParkingLot *p) {
  static int counter = 100;
  
  if (searchActive || inputActive) {
    char *activeInput = searchActive ? searchQuery : plateInput;
    int maxLen = 31;
    
    int key = GetCharPressed();
    while (key > 0) {
      if (key >= 32 && key <= 125 && strlen(activeInput) < maxLen) {
        int len = strlen(activeInput);
        activeInput[len] = (char)key;
        activeInput[len + 1] = '\0';
      }
      key = GetCharPressed();
    }
    
    if (IsKeyPressed(KEY_BACKSPACE) && strlen(activeInput) > 0) {
      activeInput[strlen(activeInput) - 1] = '\0';
    }
    
    if (searchActive) {
      search_car(p, searchQuery);
    }
    
    if (IsKeyPressed(KEY_ENTER)) {
      if (inputActive && strlen(plateInput) > 0) {
        if (is_duplicate(p, plateInput)) {
          add_notification("Car already exists!", (Color){239, 68, 68, 255});
        } else {
          int result = arrive(p, plateInput);
          
          if (result == 0) {
            for (int i = 0; i < MAX_SLOT; i++) {
              if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plateInput) == 0) {
                trigger_animation(ANIM_ARRIVE, i, plateInput);
                break;
              }
            }
          } else if (result == 1) {
            int qIdx = p->queued.curr - 1;
            if (qIdx >= 0) {
              trigger_animation(ANIM_ARRIVE, MAX_SLOT + qIdx, plateInput);
            }
          }
        }
        
        plateInput[0] = '\0';
      }
      inputActive = false;
      searchActive = false;
    }
    
    if (IsKeyPressed(KEY_ESCAPE)) {
      searchActive = false;
      inputActive = false;
      searchQuery[0] = '\0';
      searchResultSlot = -1;
    }
    
    return;
  }
  
  if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
    Vector2 mousePos = GetMousePosition();
    Rectangle searchBox = {30, 55, 250, 35};
    Rectangle inputBox = {300, 55, 220, 35};
    
    if (!CheckCollisionPointRec(mousePos, searchBox) && 
        !CheckCollisionPointRec(mousePos, inputBox)) {
      searchActive = false;
      inputActive = false;
      searchResultSlot = -1;
    }
  }

  if (IsKeyPressed(KEY_A)) {
    char plate[16];
    sprintf(plate, "CAR%03d", counter++);
    
    if (is_duplicate(p, plate)) {
      add_notification("Car already exists!", (Color){239, 68, 68, 255});
    } else {
      int result = arrive(p, plate);
      
      if (result == 0) {
        for (int i = 0; i < MAX_SLOT; i++) {
          if (p->slots[i].slot != -1 && strcmp(p->slots[i].plate, plate) == 0) {
            trigger_animation(ANIM_ARRIVE, i, plate);
            break;
          }
        }
      } else if (result == 1) {
        int qIdx = p->queued.curr - 1;
        if (qIdx >= 0) {
          trigger_animation(ANIM_ARRIVE, MAX_SLOT + qIdx, plate);
        }
      }
    }
  }

  if (IsKeyPressed(KEY_D)) {
    for (int i = 0; i < MAX_SLOT; i++) {
      if (p->slots[i].slot != -1) {
        char departPlate[32];
        strncpy(departPlate, p->slots[i].plate, 31);
        departPlate[31] = '\0';
        trigger_animation(ANIM_DEPART, i, departPlate);
        depart(p, departPlate);
        
        if (p->slots[i].slot != -1) {
          trigger_animation(ANIM_PROMOTE, i, p->slots[i].plate);
        }
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
            p->slots[i].arrival = time(NULL);
            p->filled++;
            trigger_animation(ANIM_PROMOTE, i, c.plate);
          }
          break;
        }
      }
      p->waiting = p->queued.curr;
    }
  }
}

void draw(ParkingLot *p) {
  BeginDrawing();
  
  update_animations();
  update_notifications();
  
  showTooltip = false;
  
  DrawRectangleGradientV(0, 0, screenW, screenH, 
                         (Color){243, 244, 246, 255},
                         (Color){229, 231, 235, 255});

  DrawRectangle(0, 42, screenW, 3, (Color){0, 0, 0, 30});
  DrawRectangleGradientH(0, 0, screenW, 42, 
                         (Color){37, 99, 235, 255},
                         (Color){29, 78, 216, 255});
  
  DrawText("Parking Lot Management System", 32, 12, 26, (Color){0, 0, 0, 80});
  DrawText("Parking Lot Management System", 30, 10, 26, WHITE);

  draw_search_box();
  draw_input_box();
  draw_add_buttons(p);
  
  draw_notifications();
  
  draw_stats(p);

  int spacingY = 160;
  draw_slot(p, p->slots, MAX_SLOT, "Parking Slots", spacingY, true, false);


  int queuedY = spacingY + ((MAX_SLOT + 4) / 5) * (70 + 14) + 80;
  draw_slot(p, &p->queued, MAX_QUEUE, "Waiting Queue", queuedY, false, true);

  handle_hotkeys(p);
  
  if (showTooltip) {
    draw_tooltip(currentTooltip, tooltipX, tooltipY);
  }
  
  DrawText("Search: Click search box | Entry: Type & click 'Add Car' | Random: Click 'Add Random' or press A | ESC to clear", 
           10, screenH - 20, 10, (Color){100, 116, 139, 255});

  EndDrawing();
}
