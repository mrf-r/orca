
DIR_MIDI := libs/mbwmidi

DIR_SRC_MIDI := $(DIR_MIDI)

DIRS_INCLUDE_MIDI := $(DIR_MIDI) $(DIRS_INCLUDE_FW)

SOURCES_C_MIDI := $(wildcard $(DIR_SRC_MIDI)/*.c)
# SOURCES_C_MIDI := $(DIR_SRC_MIDI)/mgl.c
# SOURCES_C_MIDI += $(DIR_SRC_MIDI)/5monotxt.c

#######################################
FLAGS_C_MIDI := $(FLAGS_C_COMMON)
FLAGS_C_MIDI += $(addprefix -I,$(DIRS_INCLUDE_MIDI))
FLAGS_C_MIDI += -Wall -Wpedantic

#######################################
OBJECTS_MIDI := $(addprefix $(DIR_OBJ)/,$(SOURCES_C_MIDI:.c=.o))
DIR_OBJ_MIDI := $(DIR_OBJ)/$(DIR_SRC_MIDI)

#######################################
$(DIR_OBJ_MIDI):
	mkdir -p $@

$(DIR_OBJ_MIDI)/%.o: $(DIR_SRC_MIDI)/%.c | $(DIR_OBJ_MIDI) 
	@echo "MIDI C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_MIDI) $< -o $@

#######################################
$(OBJECTS_MIDI): make_mgl.mk make_common.mk
include $(wildcard $(DIR_OBJ_MIDI)/*.d)

