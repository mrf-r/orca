include make_common.mk
include make_hal.mk
include make_cli.mk
#######################################
EXT_ASM := .S

DIR_SRC := src
DIRS_INCLUDE_FW := $(DIR_SRC) $(DIRS_INCLUDE_HAL) $(DIRS_INCLUDE_CLI)
#DIRS_INCLUDE_FW += app

include make_mgl.mk
include make_midi.mk

DIRS_INCLUDE_FW += $(DIR_SRC_MGL)
DIRS_INCLUDE_FW += $(DIR_SRC_MIDI)

#######################################

SOURCES_ASM_FW := $(wildcard $(DIR_SRC)/*$(EXT_ASM))
SOURCES_C_FW := $(wildcard $(DIR_SRC)/*.c)
# SOURCES_C_FW := $(filter-out $(DIR_SRC)/systeminit.c, $(SOURCES_C_FW))

TARGET_FW := ORCA_FW

ELF_FW := $(DIR_OBJ)/$(TARGET_FW).elf
BIN_FW := $(DIR_OBJ)/$(TARGET_FW).BIN
HEX_FW := $(DIR_OBJ)/$(TARGET_FW).hex

# LDSCRIPT_FW := $(DIR_SRC)/orca_aprom.ld
LDSCRIPT_FW := $(DIR_SRC)/orca_ram.ld

#######################################
FLAGS_C_FW := $(FLAGS_C_COMMON)
FLAGS_C_FW += $(addprefix -I,$(DIRS_INCLUDE_FW))
#FLAGS_C_FW += -std=gnu11
FLAGS_C_FW += -Wall -Wpedantic

FLAGS_LD_FW := $(FLAGS_LD_COMMON)
FLAGS_LD_FW += -T$(LDSCRIPT_FW)
FLAGS_LD_FW += -Xlinker -Map=$(DIR_OBJ)/$(TARGET_FW).map

#######################################
OBJECTS_FW := $(addprefix $(DIR_OBJ)/,$(SOURCES_C_FW:.c=.o))
OBJECTS_FW += $(addprefix $(DIR_OBJ)/,$(SOURCES_ASM_FW:$(EXT_ASM)=.o))
DIR_OBJ_FW := $(DIR_OBJ)/$(DIR_SRC)
#DIRS_OBJ_BOOT := $(sort $(dir $(OBJECTS_FW)))

#######################################
$(DIR_OBJ_FW):
	mkdir -p $@

$(DIR_OBJ_FW)/%.o: $(DIR_SRC)/%.c | $(DIR_OBJ_FW) $(BUILD_ID)
	@echo "FW C: $(notdir $<)"
	@$(CC) -c $(FLAGS_C_FW) $< -o $@

$(DIR_OBJ_FW)/%.o: $(DIR_SRC)/%$(EXT_ASM) | $(DIR_OBJ_FW)
	@echo "FW ASM: $(notdir $<)"
	@$(AS) -c $(FLAGS_ASM_COMMON) $< -o $@

#######################################
$(OBJECTS_FW): make_firmware.mk make_common.mk
include $(wildcard $(DIR_OBJ_FW)/*.d)

#######################################
$(ELF_FW): $(OBJECTS_FW) $(OBJECTS_HAL) $(OBJECTS_CLI) $(OBJECTS_MGL) $(OBJECTS_MIDI)
	@echo "FW elf: $(notdir $@)"
	@$(CPP) $(FLAGS_LD_FW) $^ -o $@
	@$(SZ) $@
	
$(HEX_FW): $(ELF_FW)
	@echo "FW hex: $(notdir $@)"
	@$(CP) -O ihex $< $@

$(BIN_FW): $(ELF_FW)
	@echo "FW bin: $(notdir $@)"
	@$(CP) -O binary -S $< $@

#######################################
firmware: $(ELF_FW) $(HEX_FW) $(BIN_FW)


