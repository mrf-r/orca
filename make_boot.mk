MAKE_BOOT:=

include $(MAKE_HAL)
include $(MAKE_COMMON)
#include $(MAKE_CLI)

#######################################
EXT_ASM := .S

DIR_SRC_APP := app
DIRS_INCLUDE_BOOT := $(DIR_SRC) $(DIR_SRC_APP)

SOURCES_BOOT_ASM := $(wildcard $(DIR_SRC)/*$(EXT_ASM))
SOURCES_BOOT_C := $(wildcard $(DIR_SRC)/*.c)
SOURCES_BOOT_C := \
#$(DIR_SRC)/systeminit.c \
#$(DIR_SRC)/bootloader.c \

SOURCES_BOOT_C += $(wildcard $(DIR_SRC_APP)/*.c)

TARGET_BOOT := ORCA_BOOT

ELF_BOOT := $(DIR_OBJ)/$(TARGET_BOOT).elf
BIN_BOOT := $(DIR_OBJ)/$(TARGET_BOOT).BIN
HEX_BOOT := $(DIR_OBJ)/$(TARGET_BOOT).hex

#LDSCRIPT_BOOT := $(DIR_SRC)/orca_ldrom.ld
LDSCRIPT_BOOT := $(DIR_SRC)/orca_ram.ld

#######################################
FLAGS_C_BOOT := $(FLAGS_C_COMMON)
FLAGS_C_BOOT += $(addprefix -I,$(DIRS_INCLUDE_BOOT))
FLAGS_C_BOOT += -Osize
#FLAGS_C_BOOT += -std=gnu11
FLAGS_C_BOOT += -Wall -Wpedantic
FLAGS_C_BOOT += -DBOOTLOADER

FLAGS_LD_BOOT := $(FLAGS_LD_COMMON)
FLAGS_LD_BOOT += -T$(LDSCRIPT_BOOT)
FLAGS_LD_BOOT += -Xlinker -Map=$(DIR_OBJ)/$(TARGET_BOOT).map

#######################################
OBJECTS_BOOT := $(addprefix $(DIR_OBJ)/,$(SOURCES_BOOT_C:.c=.o))
OBJECTS_BOOT += $(addprefix $(DIR_OBJ)/,$(SOURCES_BOOT_ASM:$(EXT_ASM)=.o))
DIR_OBJ_BOOT := $(DIR_OBJ)/$(DIR_SRC)
#DIRS_OBJ_BOOT := $(sort $(dir $(OBJECTS_BOOT)))

DIR_OBJ_APP := $(DIR_OBJ)/$(DIR_SRC_APP)

#######################################
$(DIR_OBJ_BOOT):
	mkdir -p $@

$(DIR_OBJ_BOOT)/%.o: $(DIR_SRC)/%.c | $(DIR_OBJ_BOOT) $(BUILD_ID)
	@echo "BL C: $(notdir $<)"
	@$(CC) -c $(FLAGS_C_BOOT) $< -o $@

$(DIR_OBJ_BOOT)/%.o: $(DIR_SRC)/%$(EXT_ASM) | $(DIR_OBJ_BOOT)
	@echo "BL ASM: $(notdir $<)"
	@$(AS) -c $(FLAGS_ASM_COMMON) $< -o $@

$(DIR_OBJ_APP):
	mkdir -p $@

$(DIR_OBJ_APP)/%.o: $(DIR_SRC_APP)/%.c | $(DIR_OBJ_APP) 
	@echo "HAL C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_HAL) $< -o $@

#######################################
$(OBJECTS_BOOT): make_boot.mk make_common.mk
include $(wildcard $(DIR_OBJ_BOOT)/*.d)

#######################################
$(ELF_BOOT): $(OBJECTS_BOOT)
	@echo "BL elf: $(notdir $@)"
	@$(CC) $(FLAGS_LD_BOOT) $^ -o $@
	@$(SZ) $@
	
$(HEX_BOOT): $(ELF_BOOT)
	@echo "BL hex: $(notdir $@)"
	@$(CP) -O ihex $< $@

$(BIN_BOOT): $(ELF_BOOT)
	@echo "BL bin: $(notdir $@)"
	@$(CP) -O binary -S $< $@

#######################################
boot: $(ELF_BOOT) $(HEX_BOOT) $(BIN_BOOT)

