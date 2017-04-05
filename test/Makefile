include ../Makefile.config

SRC_DIR = src

TARGET_DIR = out
TARGET = simulator

CDEFS = -DMCU_FREQ=$(FREQ) -DMCU_NAME=\"$(MCU)\"
CFLAGS = -g -std=gnu11 -Wall -Wextra -O3
LDFLAGS = -lpthread -lsimavr -lGL -lGLU -lglut

# ---------------------------------------------------------------------------

# Define programs and commands.
CC = gcc
REMOVE = rm -r
MKDIR = mkdir -p

# Collect source and intermediary files
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(addprefix $(TARGET_DIR)/,$(notdir $(SRC:.c=.o)))

# Compiler flags to generate dependency files.
GENDEPFLAGS = -Wp,-M,-MP,-MT,$(TARGET_DIR)/$(*F).o,-MF,$(TARGET_DIR)/$(@F).d

# Combine all necessary flags and optional flags.
ALL_CFLAGS = -I$(SRC_DIR) $(CFLAGS) $(CDEFS) $(GENDEPFLAGS)
ALL_LDFLAGS = $(LDFLAGS)

build: $(TARGET_DIR)/$(TARGET)

run: $(TARGET_DIR)/$(TARGET)
	$< -u $(UART_PORT)

debug: $(TARGET_DIR)/$(TARGET)
	$< -u $(UART_PORT) -g $(GDB_PORT)

clean:
	$(REMOVE) $(TARGET_DIR)

# Create output directory
$(TARGET_DIR):
	$(MKDIR) $(TARGET_DIR)

# Link: create binary output file from object files.
$(TARGET_DIR)/$(TARGET): $(OBJ) | $(TARGET_DIR)
	$(CC) $(ALL_CFLAGS) -o $@ $(OBJ) $(ALL_LDFLAGS)

# Compile: create object files from C source files.
$(TARGET_DIR)/%.o : $(SRC_DIR)/%.c | $(TARGET_DIR)
	$(CC) $(ALL_CFLAGS) -c -o $@ $< 

# Include the dependency files.
include $(wildcard $(TARGET_DIR)/*.d)

.PHONY: all run debug clean
.SECONDARY: $(TARGET_DIR)/$(TARGET)
.PRECIOUS: $(OBJ)

