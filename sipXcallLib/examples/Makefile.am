## Process this file with automake to produce Makefile.in

EXTRA_DIST = musicServer CallScript PlaceCall ReceiveCall

if PLACECALL
PLACECALL_DIR = PlaceCall
endif

if RECEIVECALL
RECEIVECALL_DIR = ReceiveCall
endif

if BUILDEZPHONE
SIPXEZPHONE_DIR = sipXezPhone
endif

optional_SUBDIRS = ${PLACECALL_DIR} ${RECEIVECALL_DIR} ${SIPXEZPHONE_DIR}

SUBDIRS = ${optional_SUBDIRS}
