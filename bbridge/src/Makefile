all: bbridge symlink-libs

CPPFLAGS += -Wall
CPPFLAGS += -g
CPPFLAGS += -I../include

SRC := $(wildcard *.cpp)
DEPS := $(patsubst %.cpp, .%.d, $(SRC))
OBJS := $(patsubst %.cpp, %.o, $(SRC))

######################################################################
# Add the sipX includes, libraries, and defines
SIPXPATH := ../..
LDFLAGS += -L$(SIPXPATH)/sipXmediaAdapterLib/sipXmediaMediaProcessing/src/.libs
LDFLAGS += -lsipXmediaProcessing
CPPFLAGS += -I $(SIPXPATH)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include
CPPFLAGS += -I $(SIPXPATH)/sipXmediaAdapterLib/interface

SIPXLIBS := sipXmedia sipXtack sipXport
LDFLAGS += $(patsubst %, -L$(SIPXPATH)/%Lib/src/.libs, $(SIPXLIBS))
LDFLAGS += $(patsubst %, -l%, $(SIPXLIBS))
CPPFLAGS += $(patsubst %, -I$(SIPXPATH)/%Lib/include, $(SIPXLIBS))
CPPFLAGS += -D__pingtel_on_posix__

SIPXSO := $(wildcard $(patsubst %, $(SIPXPATH)/%Lib/src/.libs/*.so*, $(SIPXLIBS)))
SIPXSO += $(wildcard $(SIPXPATH)/sipXmediaAdapterLib/sipXmediaMediaProcessing/src/.libs/*.so*)
######################################################################

bbridge: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

-include $(DEPS)

.%.d: %.cpp
	@echo Generating dependencies for $*.o
	@$(CPP) $(CPPFLAGS) -MM $< -MT $*.o -MT .$*.d > $@

.PHONY: clean symlink-libs run

symlink-libs:
	@mkdir -p libs
	@ln -fs $(patsubst %, ../%, $(SIPXSO)) libs

run: bbridge symlink-libs
	LD_LIBRARY_PATH=libs ./bbridge

clean:
	$(RM) *.a *.o .*.d
	$(RM) -r libs

# This is useful for debugging; e.g. try "make show.CXX"
show.%:
	@echo $*=$($*)

