TOP_DIR=../../../
ifdef SIPX_INST_DIR
  SIPX_BUILD_DIR_INC=-I${SIPX_INST_DIR}/include
  SIPX_BUILD_DIR_LIB=-L${SIPX_INST_DIR}/lib
else
  SIPX_BUILD_DIR_INC=-I$(TOP_DIR)/sipXmediaAdapterLib/sipXmediaMediaProcessing/include \
                     -I$(TOP_DIR)/sipXmediaAdapterLib/interface \
                     -I$(TOP_DIR)/sipXmediaLib/include \
                     -I$(TOP_DIR)/sipXportLib/include \
                     -I$(TOP_DIR)/sipXsdpLib/include \
                     -I$(TOP_DIR)/sipXtackLib/include 
  SIPX_BUILD_DIR_LIB=-L$(TOP_DIR)/sipXmediaAdapterLib/sipXmediaMediaProcessing/src/.libs \
                     -L$(TOP_DIR)/sipXmediaLib/src/.libs \
                     -L$(TOP_DIR)/sipXportLib/src/.libs \
                     -L$(TOP_DIR)/sipXsdpLib/src/.libs \
                     -L$(TOP_DIR)/sipXtackLib/src/.libs \
                     -L$(TOP_DIR)/sipXmediaLib/contrib/libspeex/libspeex/.libs
endif

INCDIRS = $(SIPX_BUILD_DIR_INC)

LIBS =  $(SIPX_BUILD_DIR_LIB) \
       -Wl,-Bstatic -lsipXmediaProcessing -lsipXmedia -lsipXsdp -lsipXport -lspeex \
       -Wl,-Bdynamic -lpcre -lcrypto -lssl

DEFINES = -D__pingtel_on_posix__

all: ${SIPX_BUILD_DIR}
	$(CXX) -Wall -g -pthread mstream.cpp -o mstream $(INCDIRS) $(LIBS) $(DEFINES)

clean:
	rm mstream

