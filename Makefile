# Wii Meta-Client Makefile
# Requires devkitPPC + libogc + GRRLIB

ifeq ($(strip $(DEVKITPPC)),)
$(error "Please set DEVKITPPC in your environment")
endif

include $(DEVKITPPC)/wii_rules

TARGET      := wii-meta-client
BUILD       := build
SOURCES     := source source/ui source/providers source/network source/storage source/utils
DATA        := data
INCLUDES    := include

CFLAGS      := -g -O2 -Wall $(MACHDEP) $(INCLUDE)
CXXFLAGS    := $(CFLAGS) -fno-rtti -fno-exceptions
LDFLAGS     := -g $(MACHDEP) -Wl,-Map,$(notdir $@).map

LIBS        := -lgrrlib -lfreetype -lpng -ljpeg -lz                -lwiiuse -lbte -lasnd -lfat -lwiikeyboard                -logc -lm

# Source files
CFILES      := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.c)))
CPPFILES    := $(foreach dir,$(SOURCES),$(notdir $(wildcard $(dir)/*.cpp)))
OFILES      := $(CPPFILES:.cpp=.o) $(CFILES:.c=.o)

VPATH       := $(foreach dir,$(SOURCES),$(CURDIR)/$(dir))

.PHONY: all clean

all: $(TARGET).dol

$(TARGET).dol: $(TARGET).elf
	$(SILENT)elf2dol $< $@

$(TARGET).elf: $(OFILES)
	$(CXX) $(LDFLAGS) -o $@ $(OFILES) $(LIBS)

clean:
	$(SILENT)rm -rf $(BUILD) $(TARGET).elf $(TARGET).dol

run: $(TARGET).dol
	$(SILENT)wiiload $(TARGET).dol
