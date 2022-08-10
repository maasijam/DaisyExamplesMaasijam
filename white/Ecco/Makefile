# Project Name
TARGET = Ecco

 DEBUG = 1
 LDFLAGS += -u _printf_float

# Sources
CPP_SOURCES = Ecco.cpp scale.cpp taptempo.cpp DelayMulti.cpp LEDs.cpp oscillator_p.cpp ../daisy_white.cpp

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

