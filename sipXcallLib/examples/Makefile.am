## Process this file with automake to produce Makefile.in

EXTRA_DIST = musicServer CallScript PlaceCall ReceiveCall

if BUILDEZPHONE
ezphone_SUBDIRS = sipXezPhone
else
ezphone_SUBDIRS = 
endif

if CALL_EXAMPLES
example_SUBDIRS = PlaceCall ReceiveCall
else
example_SUBDIRS = 
endif

SUBDIRS = ${ezphone_SUBDIRS} ${example_SUBDIRS}

DISTCLEANFILES = Makefile.in
