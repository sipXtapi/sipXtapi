
INCLUDES = -Iinclude -I$(top_srcdir)/contrib/libilbc/include

lib_LTLIBRARIES = libilbc.la

## Libtool version info - update only immediately before a release
# Current - increment if any interfaces are added, removed, or changed
version_Current=1
# Revision - set to 0 if Current was incremented
#            increment if any implementation has changed
version_Revision=0
# Age - set to 0 of any interfaces have been removed
#       increment if any interfaces have been added
version_Age=0


libilbc_la_LDFLAGS = \
        -version-info ${version_Current}:${version_Revision}:${version_Age}

libilbc_la_SOURCES = \
       src/anaFilter.c \
       src/constants.c \
       src/createCB.c \
       src/doCPLC.c \
       src/enhancer.c \
       src/filter.c \
       src/FrameClassify.c \
       src/gainquant.c \
       src/getCBvec.c \
       src/helpfun.c \
       src/hpInput.c \
       src/hpOutput.c \
       src/iCBConstruct.c \
       src/iCBSearch.c \
       src/iLBC_decode.c \
       src/iLBC_encode.c \
       src/LPCdecode.c \
       src/LPCencode.c \
       src/lsf.c \
       src/packing.c \
       src/StateConstructW.c \
       src/StateSearchW.c \
       src/syntFilter.c 
