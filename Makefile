FIRMWARE_DIR = "firmware"
FIRMWARE_TARGET = "out/firmware.elf"

SIMULATOR_DIR = "test"
SIMULATOR_TARGET = "out/simulator"

run: $(FIRMWARE_DIR)/$(FIRMWARE_TARGET) $(SIMULATOR_DIR)/$(SIMULATOR_TARGET)
	$(MAKE) -C $(SIMULATOR_DIR) run

clean:
	$(MAKE) -C $(FIRMWARE_DIR) clean
	$(MAKE) -C $(SIMULATOR_DIR) clean

$(FIRMWARE_DIR)/$(FIRMWARE_TARGET):
	$(MAKE) -C $(FIRMWARE_DIR)

$(SIMULATOR_DIR)/$(SIMULATOR_TARGET):
	$(MAKE) -C $(SIMULATOR_DIR)

.PHONY: run clean