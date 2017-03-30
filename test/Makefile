include ../Makefile.config

SIM_LIB_DIR = ../../simavr/dist

SRC = simulator.c

TARGET_DIR = out
TARGET = simulator

CDEFS = -DMCU_FREQ=$(FREQ) -DMCU_NAME=\"$(MCU)\"
CFLAGS = -I$(SIM_LIB_DIR)/include -g -std=gnu11 -Wall -Wextra -O3
LDFLAGS = -L$(SIM_LIB_DIR)/lib -lpthread -lsimavr -lGL -lGLU -lglut

CC = gcc
REMOVE = rm -r
MKDIR = mkdir -p

all: $(TARGET_DIR)/$(TARGET)

run: $(TARGET_DIR)/$(TARGET)
	LD_LIBRARY_PATH=$(SIM_LIB_DIR)/lib $<

debug: $(TARGET_DIR)/$(TARGET)
	LD_LIBRARY_PATH=$(SIM_LIB_DIR)/lib $< -g

clean:
	$(REMOVE) $(TARGET_DIR)

$(TARGET_DIR)/$(TARGET): $(SRC) | $(TARGET_DIR)
	$(CC) -o $@ $(CFLAGS) $(CDEFS) $(LDFLAGS) $<

$(TARGET_DIR):
	$(MKDIR) $(TARGET_DIR)

.PHONY: all run debug clean

