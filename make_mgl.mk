
DIR_MGL := libs/minimalgraphics

DIR_SRC_MGL := $(DIR_MGL)

DIRS_INCLUDE_MGL := $(DIR_MGL) $(DIRS_INCLUDE_FW)

#SOURCES_C_MGL := $(wildcard $(DIR_SRC_MGL)/*.c)
SOURCES_C_MGL := $(DIR_SRC_MGL)/mgl.c
SOURCES_C_MGL += $(DIR_SRC_MGL)/mgldisp_mono.c

#######################################
FLAGS_C_MGL := $(FLAGS_C_COMMON)
FLAGS_C_MGL += $(addprefix -I,$(DIRS_INCLUDE_MGL))
FLAGS_C_MGL += -Wall -Wpedantic

#######################################
OBJECTS_MGL := $(addprefix $(DIR_OBJ)/,$(SOURCES_C_MGL:.c=.o))
DIR_OBJ_MGL := $(DIR_OBJ)/$(DIR_SRC_MGL)

#######################################
$(DIR_OBJ_MGL):
	mkdir -p $@

$(DIR_OBJ_MGL)/%.o: $(DIR_SRC_MGL)/%.c | $(DIR_OBJ_MGL) 
	@echo "MGL C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_MGL) $< -o $@

#######################################
$(OBJECTS_MGL): make_mgl.mk make_common.mk
include $(wildcard $(DIR_OBJ_MGL)/*.d)

