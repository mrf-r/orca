MAKE_CLI:=

include $(MAKE_COMMON)

#######################################
DIR_CLI := libs/rtt

DIR_SRC_CLI := $(DIR_CLI)

DIRS_INCLUDE_CLI := $(DIR_CLI)

#SOURCES_C_CLI := $(wildcard $(DIR_SRC_CLI)/*.c)
SOURCES_C_CLI := $(DIR_SRC_CLI)/SEGGER_RTT.c

#######################################
FLAGS_C_CLI := $(FLAGS_C_COMMON)
FLAGS_C_CLI += $(addprefix -I,$(DIRS_INCLUDE_CLI))
FLAGS_C_CLI += -Ofast

#######################################
OBJECTS_CLI := $(addprefix $(DIR_OBJ)/,$(SOURCES_C_CLI:.c=.o))
DIR_OBJ_CLI := $(DIR_OBJ)/$(DIR_SRC_CLI)

#######################################
$(DIR_OBJ_CLI):
	mkdir -p $@

$(DIR_OBJ_CLI)/%.o: $(DIR_SRC_CLI)/%.c | $(DIR_OBJ_CLI) 
	@echo "CLI C: $(notdir $<)..."
	@$(CC) -c $(FLAGS_C_CLI) $< -o $@

#######################################
$(OBJECTS_CLI): make_cli.mk make_common.mk
include $(wildcard $(DIR_OBJ_CLI)/*.d)

