ARCH=$(PROC)
CFLAGS+=-Wall -O3 -funroll-loops -Isrc -Iinclude
ifneq (${OSARCH},CYGWIN)
CFLAGS += -fPIC
endif
LIB=lib/libilbc.a

OBJS= src/anaFilter.o \
      src/constants.o \
      src/createCB.o \
      src/doCPLC.o \
      src/enhancer.o \
      src/filter.o \
      src/FrameClassify.o \
      src/gainquant.o \
      src/getCBvec.o \
      src/helpfun.o \
      src/hpInput.o \
      src/hpOutput.o \
      src/iCBConstruct.o \
      src/iCBSearch.o \
      src/iLBC_decode.o \
      src/iLBC_encode.o \
      src/LPCdecode.o \
      src/LPCencode.o \
      src/lsf.o \
      src/packing.o \
      src/StateConstructW.o \
      src/StateSearchW.o \
      src/syntFilter.o


all: $(LIB)


$(LIB): $(OBJS)
	ar cr $(LIB) $(OBJS)
	ranlib $(LIB)

clean:
	rm -f $(LIB) *.o

install:

