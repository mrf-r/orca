MAKE_HAL:=

include $(MAKE_COMMON)

#######################################
DIR_HAL := libs/StdDriver

DIR_SRC_HAL := $(DIR_HAL)/src
DIR_SRC_APP := app

DIRS_INCLUDE_HAL := \
libs/CMSIS/Include \
libs/Device/Nuvoton/NUC123/Include \
$(DIR_HAL)/inc \
$(DIR_SRC_APP) \
#end

#SOURCES_HAL_C := $(wildcard $(DIR_SRC_HAL)/*.c)
SOURCES_HAL_C := $(DIR_SRC_HAL)/usbd.c
SOURCES_HAL_C += $(wildcard $(DIR_SRC_APP)/*.c)

#######################################
FLAGS_C_HAL := $(FLAGS_C_COMMON)
FLAGS_C_HAL += $(addprefix -I,$(DIRS_INCLUDE_HAL))
FLAGS_C_HAL += -Ofast

#######################################
OBJECTS_HAL := $(addprefix $(DIR_OBJ)/,$(SOURCES_HAL_C:.c=.o))
DIR_OBJ_HAL := $(DIR_OBJ)/$(DIR_SRC_HAL)

DIR_OBJ_APP := $(DIR_OBJ)/$(DIR_SRC_APP)

#######################################
$(DIR_OBJ_HAL):
	mkdir -p $@

$(DIR_OBJ_HAL)/%.o: $(DIR_SRC_HAL)/%.c | $(DIR_OBJ_HAL) 
	@echo "HAL C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_HAL) $< -o $@

$(DIR_OBJ_APP):
	mkdir -p $@

$(DIR_OBJ_APP)/%.o: $(DIR_SRC_APP)/%.c | $(DIR_OBJ_APP) 
	@echo "HAL C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_HAL) $< -o $@

#######################################
$(OBJECTS_HAL): make_hal.mk make_common.mk
include $(wildcard $(DIR_OBJ_HAL)/*.d)

