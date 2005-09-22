all: bbridge symlink-libs

CPPFLAGS += -Wall
CPPFLAGS += -g
CPPFLAGS += -I../include

SRC := $(wildcard *.cpp)
DEPS := $(patsubst %.cpp, .%.d, $(SRC))
OBJS := $(patsubst %.cpp, %.o, $(SRC))

include Makefile.config

UNAME_OS := `uname`
UNAME_P  := `uname -p`

######################################################################
# Add the sipX includes, libraries, and defines
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

######################################################################
# Add the resip includes, libraries, and defines
LDFLAGS += -L$(RESIPPATH)/resip/dum/$(RESIPOBJ)/ -ldum
LDFLAGS += -L$(RESIPPATH)/resip/stack/$(RESIPOBJ)/ -lresip
LDFLAGS += -L$(RESIPPATH)/rutil/$(RESIPOBJ)/ -lrutil
LDFLAGS += -L$(RESIPPATH)/contrib/ares -lares
CPPFLAGS += -I$(RESIPPATH)
######################################################################

bbridge: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

-include $(DEPS)

Makefile.config:
	@echo "SIPXPATH  := ../.." > Makefile.config
	@echo "RESIPPATH := ../../../resiprocate" >> Makefile.config
	@echo "RESIPOBJ  := obj.debug.$(UNAME_OS).$(UNAME_P)" >> Makefile.config
	@echo ""
	@echo "*****************************************************"
	@echo "*****************************************************"
	@echo "*** NOTE: A Makefile.config file has been created ***"
	@echo "*** for you. Check the settings in that file and  ***"
	@echo "*** verify that they make sense for your          ***"
	@echo "*** configuration                                 ***"
	@echo "*****************************************************"
	@echo "*****************************************************"
	@echo ""

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

