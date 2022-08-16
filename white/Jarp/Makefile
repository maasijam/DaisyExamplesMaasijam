# Project Name
TARGET = Jarp

# Sources
CPP_SOURCES = Jarp.cpp \
lib/ev_theory/theory.cpp \
lib/ev_theory/arp.cpp \
lib/ev_theory/chord.cpp \
lib/ev_theory/rhythm.cpp \
../daisy_white.cpp

# Electrosmith library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Compiler flag to optimize for binary size. Reduces bin size by ~10%
OPT = -Os

# Core location and generic Makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile	