#
#  Copyright (c) 2015, Russell
#  All rights reserved.
#
#  Redistribution and use in source and binary forms, with or without
#  modification, are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
#  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
#  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
#  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
#  FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
#  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
#  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
#  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
#  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#

CC := $(CC)
CFLAGS := -c -O2 -Wall $(CFLAGS)
LDFLAGS := -O2 -Wall $(LDFLAGS)

SRCDIR := .
OBJDIR := obj

CFLAGS += -I$(SRCDIR)/interface

SRCS := $(notdir $(shell find $(SRCDIR) ! -name 'decoder.c' -a ! -name 'coder.c' -a -name '*.c'))
OBJS := $(addprefix $(OBJDIR)/, $(patsubst %.c, %.o, $(SRCS)))
DEPS := $(OBJS:.o=.d)

CODEROBJ   := coder.o
DECODEROBJ := decoder.o

-include $(DEPS)

EXECUTABLEENCODER := coder
EXECUTABLEDECODER := decoder

.PHONY: all
all : $(EXECUTABLEENCODER) $(EXECUTABLEDECODER)

$(OBJDIR)/%.o : $(SRCDIR)/%.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, %.d, $@) -o $@ $<

$(OBJDIR)/$(CODEROBJ) : $(SRCDIR)/coder.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, %.d, $@) -o $@ $<
	
$(OBJDIR)/$(DECODEROBJ) : $(SRCDIR)/decoder.c
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -MMD -MF $(patsubst %.o, %.d, $@) -o $@ $<

$(EXECUTABLEENCODER) : $(OBJDIR)/$(CODEROBJ) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

$(EXECUTABLEDECODER) : $(OBJDIR)/$(DECODEROBJ) $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

.PHONY: clean
clean:
	@rm -rf $(OBJDIR)
	@rm -f $(EXECUTABLEENCODER) $(EXECUTABLEDECODER)
