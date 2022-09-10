MAKE_COMMON:=

#######################################
ifndef GCC_PATH
GCC_PATH = /usr/bin
endif
CC := '$(GCC_PATH)/arm-none-eabi-gcc'
CPP:= '$(GCC_PATH)/arm-none-eabi-g++'
AS := '$(GCC_PATH)/arm-none-eabi-as'
CP := '$(GCC_PATH)/arm-none-eabi-objcopy'
SZ := '$(GCC_PATH)/arm-none-eabi-size'

ifndef BUILD_TAG
BUILD_TAG := loc_undef
endif

#######################################
DIR_SRC := src
DIR_OBJ := obj
BUILD_ID := $(DIR_OBJ)/signature.h

#######################################
FLAGS_MCU := -mcpu=cortex-m0 -mthumb

DEFINES_C_COMMON := 

ifeq ($(DEBUG),1)
DEFINES_C_COMMON += DEBUG
else
DEFINES_C_COMMON += NDEBUG
endif

FLAGS_ASM_COMMON := $(FLAGS_MCU) -gdwarf-2 --fatal-warnings 
FLAGS_C_COMMON := $(FLAGS_MCU)
FLAGS_C_COMMON += $(addprefix -D,$(DEFINES_C_COMMON))
FLAGS_C_COMMON += -gdwarf-2 -fdata-sections -ffunction-sections
FLAGS_C_COMMON += -MMD -MP
FLAGS_C_COMMON += -ffast-math
#FLAGS_C_COMMON += -flto

ifeq ($(DEBUG),1)
FLAGS_C_COMMON += -g3
else
FLAGS_C_COMMON += -g0
endif

# link by g++, arg pass thru -Xlinker
FLAGS_LD_COMMON := $(FLAGS_MCU)
FLAGS_LD_COMMON += -lm -lc -lgcc
FLAGS_LD_COMMON += -specs=nano.specs -specs=nosys.specs
#FLAGS_LD_COMMON += --cref # add cross reference to map file
FLAGS_LD_COMMON += -Xlinker --gc-sections
FLAGS_LD_COMMON += -Xlinker --print-memory-usage
#FLAGS_LD_COMMON += -flto

#######################################
.BUILD_SIGN:

$(BUILD_ID): .BUILD_SIGN | $(DIR_OBJ)
	@bash build_signature.sh $@ $(BUILD_TAG)





