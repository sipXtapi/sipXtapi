## Process this file with automake to produce Makefile.in

EXTRA_DIST = CallScript PlaceCall ReceiveCall

if BUILDEZPHONE
optional_SUBDIRS = sipXezPhone
else
optional_SUBDIRS = 
endif

SUBDIRS = musicServer ${optional_SUBDIRS}
