# Project Name
TARGET = Drone

DEBUG = 1
LDFLAGS += -u _printf_float

# Sources
CPP_SOURCES = Drone.cpp ../daisy_saul.cpp

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP
STMLIB_DIR = ../../stmlib

C_INCLUDES += \
-I. \
-Iresources \
-I../..

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

