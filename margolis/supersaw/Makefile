# Project Name
TARGET = supersaw

 #DEBUG = 1
 #LDFLAGS += -u _printf_float
 #APP_TYPE = BOOT_NONE
USE_FATFS = 1

APP_TYPE = BOOT_SRAM

# Sources
CPP_SOURCES = supersaw.cpp ui.cpp ../daisy_margolis.cpp 

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

