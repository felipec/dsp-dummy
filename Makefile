CROSS_COMPILE ?= arm-linux-
CC := $(CROSS_COMPILE)gcc

CFLAGS := -O2 -Wall -Wextra -Wno-unused-parameter -std=c99
LDFLAGS := -Wl,--as-needed

override CFLAGS += -D_GNU_SOURCE

DSP_API := 1

override CFLAGS += -DDSP_API=$(DSP_API)

ifdef DEBUG
  override CFLAGS += -DDEBUG
endif

CL6X := $(DSP_TOOLS)/bin/cl6x
LNK6X := $(DSP_TOOLS)/bin/lnk6x
DLLCREATE := $(DSP_DOFFBUILD)/bin/DLLcreate

all:

# dummy

dummy: dummy_arm.o dsp_bridge.o log.o

bins += dummy

dummy.x64P: dummy_dsp.o64P dummy_bridge.o64P

dummy.dll64P: dummy.x64P
dummy.dll64P: override CFLAGS := -I$(DSP_TOOLS)/include

bins += dummy.dll64P

all: $(bins)

# pretty print
ifndef V
QUIET_CC    = @echo '   CC         '$@;
QUIET_LINK  = @echo '   LINK       '$@;
QUIET_CLEAN = @echo '   CLEAN      '$@;
QUIET_DLL   = @echo '   DLLCREATE  '$@;
endif

%.o64P:: %.s
	$(QUIET_CC)$(CL6X) $(CFLAGS) -mv=64p -eo.o64P -c $<

%.o64P:: %.c
	$(QUIET_CC)$(CL6X) $(CFLAGS) -mv=64p -eo.o64P -c $<

%.x64P::
	$(QUIET_LINK)$(LNK6X) -r -cr --localize='$$bss' -o $@ $+

%.dll64P::
	$(QUIET_DLL)$(DLLCREATE) $< -o=$@

%.o:: %.c
	$(QUIET_CC)$(CC) $(CFLAGS) -MMD -o $@ -c $<

dummy:
	$(QUIET_LINK)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(QUIET_CLEAN)$(RM) $(bins) *.o *.o64P *.x64P

-include *.d
