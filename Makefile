CC := arm-linux-gcc
# CFLAGS := -Wall -g3
CFLAGS := -Wall -O2

CL6X := $(DSP_TOOLS)/bin/cl6x
LNK6X := $(DSP_TOOLS)/bin/lnk6x
DLLCREATE := $(DSP_DOFFBUILD)/bin/DLLcreate

ifdef DEBUG
CFLAGS += -DDEBUG
endif

# bridge
BRIDGE_LIBS := -L$(BRIDGE_DIR) -lbridge
BRIDGE_CFLAGS := -I$(BRIDGE_DIR)/inc -DOMAP_3430

all:

# dummy

dummy: dummy_arm.o log.o
dummy: CFLAGS := $(CFLAGS) $(BRIDGE_CFLAGS)
dummy: LIBS := $(BRIDGE_LIBS) -lpthread

bins += dummy

dummy.x64P: dummy_dsp.o64P dummy_bridge.o64P

dummy.dll64P: dummy.x64P
dummy.dll64P: CFLAGS :=
dummy.dll64P: INCLUDES := -I$(DSP_TOOLS)/include

bins += dummy.dll64P

all: $(bins)

clean:
	$(Q)rm -f $(bins)
	$(Q)rm -f *.o *.o64P *.x64P

# from Lauri Leukkunen's build system
ifdef V
Q = 
P = @printf "" # <- space before hash is important!!!
else
P = @printf "[%s] $@\n" # <- space before hash is important!!!
Q = @
endif

%.o64P:: %.s
	$(P)CL6X
	$(Q)$(CL6X) $(CFLAGS) $(INCLUDES) -mv=64p -eo.o64P -c $<

%.o64P:: %.c
	$(P)CL6X
	$(Q)$(CL6X) $(CFLAGS) $(INCLUDES) -mv=64p -eo.o64P -c $<

%.x64P::
	$(P)LNK6X
	$(Q)$(LNK6X) -r -cr --localize='$$bss' -o $@ $+

%.dll64P::
	$(P)DLLCREATE
	$(Q)$(DLLCREATE) $< -o=$@

%.o:: %.c
	$(P)CC
	$(Q)$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $<

dummy:
	$(P)LINK
	$(Q)$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)
