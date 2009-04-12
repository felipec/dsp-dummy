CC := arm-linux-gcc
CFLAGS := -O2 -Wall -Werror -ansi -std=c99

CL6X := $(DSP_TOOLS)/bin/cl6x
LNK6X := $(DSP_TOOLS)/bin/lnk6x
DLLCREATE := $(DSP_DOFFBUILD)/bin/DLLcreate

ifdef DEBUG
CFLAGS += -DDEBUG
endif

all:

# dummy

dummy: dummy_arm.o dsp_bridge.o log.o

bins += dummy

dummy.x64P: dummy_dsp.o64P dummy_bridge.o64P

dummy.dll64P: dummy.x64P
dummy.dll64P: CFLAGS :=
dummy.dll64P: INCLUDES := -I$(DSP_TOOLS)/include

bins += dummy.dll64P

all: $(bins)

clean:
	$(QUIET_CLEAN)$(RM) $(bins) *.o *.o64P *.x64P

# pretty print
V = @
Q = $(V:y=)
QUIET_CC    = $(Q:@=@echo '   CC         '$@;)
QUIET_LINK  = $(Q:@=@echo '   LINK       '$@;)
QUIET_CLEAN = $(Q:@=@echo '   CLEAN      '$@;)
QUIET_DLL   = $(Q:@=@echo '   DLLCREATE  '$@;)

%.o64P:: %.s
	$(QUIET_CC)$(CL6X) $(CFLAGS) $(INCLUDES) -mv=64p -eo.o64P -c $<

%.o64P:: %.c
	$(QUIET_CC)$(CL6X) $(CFLAGS) $(INCLUDES) -mv=64p -eo.o64P -c $<

%.x64P::
	$(QUIET_LINK)$(LNK6X) -r -cr --localize='$$bss' -o $@ $+

%.dll64P::
	$(QUIET_DLL)$(DLLCREATE) $< -o=$@

%.o:: %.c
	$(QUIET_CC)$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

dummy:
	$(QUIET_CC)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
