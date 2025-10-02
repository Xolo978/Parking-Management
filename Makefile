CC = clang
CFLAGS = -g -Wall -O0
INCLUDES = -Iinclude
LIBS = -lraylib -lm -ldl -lpthread -lGL -lX11 -lrt
TARGET = parking_lot
SRCS = src/main.c src/gui.c src/parking.c
OBJS = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
