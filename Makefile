include make_firmware.mk

.DEFAULT help:
	@echo "run: make boot|firmware|emulator [DEBUG=1]"
	@echo "example: make boot DEBUG=1 - make debuggable bl"
	@echo "example: make firmware - make final firmware"
	@echo $(DIR_OBJ)

clean:
	-rm -fR $(DIR_OBJ)

all:
	@$(MAKE) --no-print-directory clean
	@$(MAKE) --no-print-directory firmware

#######################################
# emulator ?
emulator:
	@echo "emu not ready yet"
