# ARCADE3D - four fixed-camera 3D games in one PS2 ELF.
#
# Set PS2DEV to your ps2dev installation if it is not in /usr/local/ps2dev.
# Example: make PS2DEV=$HOME/ps2dev

PS2DEV ?= /usr/local/ps2dev
PS2SDK ?= $(PS2DEV)/ps2sdk
GSKIT  ?= $(PS2DEV)/gsKit

EE_PREFIX := $(PS2DEV)/ee/bin/mips64r5900el-ps2-elf-
CC      := $(EE_PREFIX)gcc
STRIP   := $(EE_PREFIX)strip
HOST_CC ?= gcc

TARGET  := ARCADE3D.ELF
OBJS    := main_ps2.o app.o games.o gfx3d.o font3d.o
HOSTBIN := build/preview

INCS := -I. -I$(PS2SDK)/ee/include -I$(PS2SDK)/common/include -I$(GSKIT)/include
CFLAGS := -D_EE -G0 -O2 -Wall -Wextra -std=gnu99 $(INCS)
LDFLAGS := -T$(PS2SDK)/ee/startup/linkfile -L$(PS2SDK)/ee/lib -L$(GSKIT)/lib -Wl,-zmax-page-size=128
LIBS := -Wl,--start-group -lgskit -ldmakit -lgskit_toolkit -lpad -lkernel -lcglue -lc -lm -lgcc -Wl,--end-group

HOST_SRCS := main_host.c app.c games.c gfx3d.c font3d.c
HOST_CFLAGS := -DHOST_SVG -O2 -Wall -Wextra -std=gnu99 -I.

.PHONY: all clean preview run-preview zip
all: $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS) $(LIBS)
	$(STRIP) --strip-all $@
	@ls -lh $@
	@file $@

preview: $(HOSTBIN)
	mkdir -p previews
	./$(HOSTBIN)

$(HOSTBIN): $(HOST_SRCS) $(wildcard *.h)
	mkdir -p build
	$(HOST_CC) $(HOST_CFLAGS) -o $@ $(HOST_SRCS) -lm

run-preview: preview
	@echo "SVG previews are in ./previews"

zip: $(TARGET)
	zip -r -q ps2-3d-arcade.zip $(TARGET) README.md Makefile *.c *.h previews
	@ls -lh ps2-3d-arcade.zip

clean:
	rm -f $(TARGET) $(OBJS) $(HOSTBIN) ps2-3d-arcade.zip
	rm -rf build
