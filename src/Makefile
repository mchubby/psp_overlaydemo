TARGET = overlaydemo
OBJS = main.o exports.o
LIBS = 
LIBS += libcmlibMenu.a
WARN_FLAGS=-Wall
CFLAGS = -Os -G0 -fno-strict-aliasing $(WARN_FLAGS)
ASFLAGS = $(CFLAGS)

USE_KERNEL_LIBC = 1
USE_KERNEL_LIBS = 1

PSP_FW_VERSION=620

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build_prx.mak

