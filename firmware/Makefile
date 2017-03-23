MCU = atmega328p
FREQ = 8000000

FORMAT = ihex

SRC_DIR = src
TARGET_DIR = out
TARGET = firmware

CDEFS = -DF_CPU=$(FREQ) 
CFLAGS = -std=gnu99 -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums -Wall -Wextra -Wstrict-prototypes -g -Os -Wa,-adhlns=$(<:$(SRC_DIR)/%.c=$(TARGET_DIR)/%.lst)
LDFLAGS = -Wl,-Map=$(TARGET_DIR)/$(TARGET).map,--cref -lm

AVRDUDE_PROGRAMMER = usbtiny
AVRDUDE_PORT = com1    # programmer connected to serial device

# ---------------------------------------------------------------------------

# Define programs and commands.
CC = avr-gcc
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
REMOVE = rm -r
MKDIR = mkdir -p

SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(addprefix $(TARGET_DIR)/,$(notdir $(SRC:.c=.o)))

# Compiler flags to generate dependency files.
GENDEPFLAGS = -Wp,-M,-MP,-MT,$(TARGET_DIR)/$(*F).o,-MF,$(TARGET_DIR)/$(@F).d

# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = -mmcu=$(MCU) -I$(SRC_DIR) $(CFLAGS) $(CDEFS) $(GENDEPFLAGS)

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep
AVRDUDE_FLAGS = -p $(MCU) -P $(AVRDUDE_PORT) -c $(AVRDUDE_PROGRAMMER)

# ---------------------------------------------------------------------------

# Default target.
all: build elfsize

build: $(TARGET_DIR)/$(TARGET).elf $(TARGET_DIR)/$(TARGET).hex $(TARGET_DIR)/$(TARGET).eep $(TARGET_DIR)/$(TARGET).lss $(TARGET_DIR)/$(TARGET).sym

# Display size of file.
size: elfsize hexsize

elfsize: $(TARGET_DIR)/$(TARGET).elf 
	$(SIZE) -A $<

hexsize: $(TARGET_DIR)/$(TARGET).hex $(TARGET_DIR)/$(TARGET).eep
	$(SIZE) --target=$(FORMAT) $^

# Program the device.  
program: 
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH) $(AVRDUDE_WRITE_EEPROM)

program_flash: $(TARGET_DIR)/$(TARGET).hex $(TARGET_DIR)/$(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)

program_eeprom: $(TARGET_DIR)/$(TARGET).hex $(TARGET_DIR)/$(TARGET).eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_EEPROM)

# Target: clean project.
clean: 
	$(REMOVE) $(TARGET_DIR)

# ---------------------------------------------------------------------------

$(TARGET_DIR):
	$(MKDIR) $(TARGET_DIR)

# Create final output files (.hex, .eep) from ELF output file.
$(TARGET_DIR)/%.hex: $(TARGET_DIR)/%.elf | $(TARGET_DIR)
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

$(TARGET_DIR)/%.eep: $(TARGET_DIR)/%.elf | $(TARGET_DIR)
	$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" --change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
$(TARGET_DIR)/%.lss: $(TARGET_DIR)/%.elf | $(TARGET_DIR)
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
$(TARGET_DIR)/%.sym: $(TARGET_DIR)/%.elf | $(TARGET_DIR)
	$(NM) -n $< > $@

# Link: create ELF output file from object files.
$(TARGET_DIR)/%.elf: $(OBJ) | $(TARGET_DIR)
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)

# Compile: create object files from C source files.
$(TARGET_DIR)/%.o : $(SRC_DIR)/%.c | $(TARGET_DIR)
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 

# Include the dependency files.
include $(wildcard $(TARGET_DIR)/*.d)

# Listing of phony targets.
.PHONY : all size elfsize hexsize build elf hex eep lss sym clean program program_flash program_eeprom
.SECONDARY : $(TARGET_DIR)/$(TARGET).elf
.PRECIOUS : $(OBJ)

