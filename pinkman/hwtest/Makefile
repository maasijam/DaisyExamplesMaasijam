# Project Name
TARGET = hwtest

 #DEBUG = 1
 #LDFLAGS += -u _printf_float
 #APP_TYPE = BOOT_NONE
 #USE_FATFS = 1

APP_TYPE = BOOT_NONE

# Sources
CPP_SOURCES = hwtest.cpp settings.cpp ui.cpp ../daisy_pinkman.cpp

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

