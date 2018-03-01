

CXX=g++
CPP=gcc
LD=ld
UNAME:=$(shell uname)

FFMPEG_CFLAGS=-D_ISOC99_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE_SOURCE -D_POSIX_C_SOURCE=200112 -D_XOPEN_SOURCE=600 -DHAVE_AV_CONFIG_H -std=c99 -pthread -g -Wdeclaration-after-statement -Wall -Wno-parentheses -Wno-switch -Wdisabled-optimization -Wpointer-arith -Wredundant-decls -Wno-pointer-sign -Wcast-qual -Wwrite-strings -Wundef -Wmissing-prototypes -Wno-pointer-to-int-cast  -fno-math-errno -fno-tree-vectorize 

INCLUDE_FLAGS= -I../../../../contrib/ffmpeg -I../../../../include -I../../../../../sipXportLib/include -I../../../../contrib/ffmpeg/libavcodec

#LDFLAGS=-lm  ../../../../contrib/ffmpeg/libavcodec/libavcodec.a ../../../../contrib/ffmpeg/libavutil/libavutil.a
#LDFLAGS=-lm  -L../../../../contrib/ffmpeg/libavcodec -lavcodec -L../../../../contrib/ffmpeg/libavutil -lavutil
LDFLAGS= -L../../../.libs -lsipXmedia -lm -Wl,-rpath=/usr/local/lib -lavcodec -lavutil
# -Wl,--whole-archive 
LIB_DIR=../../../../bin
OBJ_DIR=obj

ifeq ($(UNAME),Darwin)
LIBEXT=dylib
SHLIB=$(CPP) dynamiclib -Wl,--no-undefined
PLATFORM_CFLAGS=-I/sw/include
endif
ifeq ($(UNAME),Linux)
LIBEXT=so
SHLIB=$(CPP) -shared -Wl,--no-undefined
PLATFORM_CFLAGS=
endif

CFLAGS+= -O2 -g -fPIC -D__pingtel_on_posix__
CFLAGS+= $(PLATFORM_CFLAGS) $(FFMPEG_CFLAGS) $(INCLUDE_FLAGS)

SRC_PLGAAC_SOURCES = plgaac.c
SRC_PLGAAC_OBJECTS= $(addprefix $(OBJ_DIR)/, $(SRC_PLGAAC_SOURCES:.c=.o))

codec_aac: foo $(LIB_DIR)/codec_aac.$(LIBEXT)

foo:
	echo src: $(SRC_PLGAAC_SOURCES)
	echo objs: $(SRC_PLGAAC_OBJECTS)

clean:
	rm -rf $(OBJ_DIR)

$(LIB_DIR)/codec_aac.$(LIBEXT): $(SRC_PLGAAC_OBJECTS)
	$(SHLIB) $^ $(LDFLAGS) -o $@

$(OBJ_DIR)/%.o: %.c
	if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	$(CPP) $(CFLAGS) -o $@ -c $<

-include $(OBJ_DIR)/.deps
$(OBJ_DIR)/.deps: $(SRC_PLGAAC_SOURCES)
	if [ ! -d $(OBJ_DIR) ] ; then mkdir -p $(OBJ_DIR) ; fi
	$(CPP) $(CFLAGS) -MM $^ | sed -e 's/^\([^ ]\)/$$(OBJ_DIR)\/\1/' > $(OBJ_DIR)/.deps

install: codec_aac
	/usr/bin/install -c $(LIB_DIR)/codec_aac.$(LIBEXT) /usr/local/sipx/share/sipxmedialib/codec_aac.$(LIBEXT)
	PATH="$(PATH):/sbin" ldconfig -n /usr/local/sipx/share/sipxmedialib


