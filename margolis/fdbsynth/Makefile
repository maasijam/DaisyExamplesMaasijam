# Project Name
TARGET = fdbsynth

 #DEBUG = 1
 #LDFLAGS += -u _printf_float



# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP
CMSIS_DSP_SRC_DIR = ${LIBDAISY_DIR}/Drivers/CMSIS/DSP/Source

C_DEFS = -DTARGET_DAISY

C_SOURCES = \
	${CMSIS_DSP_SRC_DIR}/CommonTables/arm_common_tables.c \
	${CMSIS_DSP_SRC_DIR}/ControllerFunctions/arm_sin_cos_f32.c

# Sources
CPP_SOURCES = \
	fdbsynth.cpp \
	../daisy_margolis.cpp \
	src/BiquadFilters.cpp \
	src/FeedbackSynthControls.cpp \
	src/FeedbackSynthEngine.cpp \
	src/KarplusString.cpp \
	src/memory/sdram_alloc.cpp

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

