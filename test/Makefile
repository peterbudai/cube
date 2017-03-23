TARGET_DIR = out
TARGET = simulator

CFLAGS = -I../../simavr/dist/include -g -std=c11 -Wall -Wextra -O3
LDFLAGS = -L../../simavr/dist/lib -lpthread -lsimavr -lGL -lGLU -lglut

CC = gcc
REMOVE = rm -r
MKDIR = mkdir -p

all: $(TARGET_DIR)/$(TARGET)

run: $(TARGET_DIR)/$(TARGET)
	LD_LIBRARY_PATH=../../simavr/dist/lib $< 

clean:
	$(REMOVE) $(TARGET_DIR)

$(TARGET_DIR)/$(TARGET): simulator.c | $(TARGET_DIR)
	$(CC) -o $@ $(CFLAGS) $(LDFLAGS) $<

$(TARGET_DIR):
	$(MKDIR) $(TARGET_DIR)

.PHONY: all run clean

