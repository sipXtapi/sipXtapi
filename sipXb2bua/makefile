IDIR =include -Icontrib/include
CC=gcc
CFLAGS=-I$(IDIR) -D__pingtel_on_posix__

ODIR=obj
LDIR =contrib/lib/i386-linux

LIBS=-lm -L$(LDIR) -lsipXport -lsipXcall -lsipXmedia -lsipXmediaProcessing -lsipXtack -lstdc++ -lpcre -lpthread -lssl -lsipXport -lsipXcall -lsipXmedia -lsipXport


_OBJ = \
    B2BUA.o \
    LineContext.o \
    TempLineContext.o \
    LineContextManager.o \
    CallContext.o \
    CallContextManager.o \
    ConfContext.o \
    ConfContextManager.o

OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: src/%.cpp 
	$(CC) -c -o $@ $< $(CFLAGS)

B2BUA: $(OBJ)
	gcc -o $@ $^ $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
