ROOTDIR = $(N64_INST)
GCCN64PREFIX = $(ROOTDIR)/bin/mips64-elf-
CHKSUM64PATH = $(ROOTDIR)/bin/chksum64
MKDFSPATH = $(ROOTDIR)/bin/mkdfs
N64TOOL = $(ROOTDIR)/bin/n64tool

HEADERNAME = header.ed64
HEADERTITLE = "ED64Plus OS"

SRCDIR = ./src
INCDIR = ./inc
RESDIR = ./res
OBJDIR = ./obj
BINDIR = ./bin
EXTDIR = ./external

LINK_FLAGS = -O3 -L$(ROOTDIR)/lib -L$(ROOTDIR)/mips64-elf/lib --start-group -ldragon -lmad -lc -lm -ldragonsys -lnosys --end-group -Tn64ld.x
PROG_NAME = OS64P
CFLAGS = -std=gnu99 -march=vr4300 -mtune=vr4300 -O3 -G0 -DNDEBUG -I$(INCDIR) -I$(EXTDIR)/stb -I$(EXTDIR)/inih -I$(ROOTDIR)/include -I$(ROOTDIR)/mips64-elf/include $(INIH_FLAGS) $(SET_DEBUG)
# inih settings (submodule external/inih)
INIH_FLAGS = -DINI_MAX_LINE=200 -DINI_ALLOW_MULTILINE=1 -DINI_ALLOW_BOM=1 -DINI_USE_STACK=1
ASFLAGS = -mtune=vr4300 -march=vr4300
CC = $(GCCN64PREFIX)gcc
AS = $(GCCN64PREFIX)as
LD = $(GCCN64PREFIX)ld
OBJCOPY = $(GCCN64PREFIX)objcopy

SOURCES := $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o) $(OBJDIR)/ini.o

$(PROG_NAME).v64: $(PROG_NAME).elf $(PROG_NAME).dfs
	$(OBJCOPY) $(BINDIR)/$(PROG_NAME).elf $(BINDIR)/$(PROG_NAME).bin -O binary
	rm -f $(BINDIR)/$(PROG_NAME).v64
	$(N64TOOL) -l 4M -t $(HEADERTITLE) -h $(RESDIR)/$(HEADERNAME) -o $(BINDIR)/$(PROG_NAME).v64 $(BINDIR)/$(PROG_NAME).bin -s 1M $(BINDIR)/$(PROG_NAME).dfs
	$(CHKSUM64PATH) $(BINDIR)/$(PROG_NAME).v64

$(PROG_NAME).elf : $(OBJECTS)
	@mkdir -p $(BINDIR)
	$(LD) -o $(BINDIR)/$(PROG_NAME).elf $(OBJECTS) $(LINK_FLAGS)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/ini.o : $(EXTDIR)/inih/ini.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG_NAME).dfs:
	@mkdir -p $(BINDIR)
	$(MKDFSPATH) $(BINDIR)/$(PROG_NAME).dfs $(RESDIR)/filesystem/

.PHONY: all debug clean

all: $(PROG_NAME).v64

debug: $(PROG_NAME).v64

debug: SET_DEBUG=-DDEBUG

clean:
	rm -f $(BINDIR)/*.v64 $(BINDIR)/*.elf $(OBJDIR)/*.o $(BINDIR)/*.bin $(BINDIR)/*.dfs
