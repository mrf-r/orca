MAKE_COMMON := make_common.mk
MAKE_HAL := make_hal.mk
MAKE_CLI := make_cli.mk
MAKE_BOOT := make_boot.mk
MAKE_FIRMWARE := make_firmware.mk

.DEFAULT help:
	@echo "run: make boot|firmware|emulator [DEBUG=1]"
	@echo "example: make boot DEBUG=1 - make debuggable bl"
	@echo "example: make firmware - make final firmware"

clean:
	-rm -fR $(DIR_OBJ) $(BUILD_ID)

all:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory firmware
#@$(MAKE) --no-print-directory boot firmware
#######################################
# bootloader
include $(MAKE_BOOT)

#######################################
# firmware
include $(MAKE_FIRMWARE)

#######################################
# emulator ?
emulator:
	@echo "emu not ready yet"
