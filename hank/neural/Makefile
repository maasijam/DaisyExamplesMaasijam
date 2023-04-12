# Project Name
TARGET = neuralseed

 #DEBUG = 1
 #LDFLAGS += -u _printf_float

# Sources
CPP_SOURCES = neuralseed.cpp ../daisy_hank.cpp

# Library Locations
LIBDAISY_DIR = ../../libdaisy
DAISYSP_DIR = ../../DaisySP

# Core location, and generic makefile.
SYSTEM_FILES_DIR = $(LIBDAISY_DIR)/core
include $(SYSTEM_FILES_DIR)/Makefile

#C_INCLUDES += -I../Terrarium -I../RTNeural
C_INCLUDES +=  -I../RTNeural
CPPFLAGS += -DRTNEURAL_DEFAULT_ALIGNMENT=8 -DRTNEURAL_NO_DEBUG=1