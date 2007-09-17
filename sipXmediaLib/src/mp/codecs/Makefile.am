## Process this file with automake to produce Makefile.in

SUBDIRS = \
	@PCMAPCMU_TARGET@ \
	@GSM_TARGET@ \
	@SPEEX_TARGET@ \
	@ILBC_TARGET@ \
	@TONES_TARGET@ \
	@G726_TARGET@ \
	@G722_TARGET@ \
	@AMR_TARGET@ \
	@AMRWB_TARGET@

all: install-codec

install-codec: all-recursive
	for I in $(SUBDIRS); do echo Installing plugin $$I ...; \
	    cp -f $(PWD)/$$I/.libs/libcodec*.so @DEFAULT_CODECS_PATH@ ; \
	done


clean: clean-codec

clean-codec:
	rm -f @DEFAULT_CODECS_PATH@/libcodec*.so