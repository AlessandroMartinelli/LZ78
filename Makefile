PROGS = lz78

#CFLAGS += -Werror 
CFLAGS = -Wall -g -Wunused-function
CFLAGS += -Wextra
LDFLAGS = -lm # math.h is not a standard lib, we need to include it
LDFLAGS += -lcrypto #for md5 checksum 
#CFLAGS += -DDO_STAT

CFLDR = source/common/
EFLDR = source/encoder/
DFLDR = source/decoder/
BUILDFLDR = build/
SRCS = $(addprefix $(CFLDR), header.c util.c bitio.c)
SRCS += $(addprefix $(EFLDR), hash_table.c comp.c)
SRCS += $(DFLDR)decomp.c
SRCS += source/lz78.c
OBJS= $(SRCS:%.c=%.o)
CLEANFILES = $(PROGS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)
	test -d $(BUILDFLDR) || mkdir $(BUILDFLDR)
	mv -t $(BUILDFLDR) $(CLEANFILES)

lz78: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

source/lz78.o:	$(CFLDR)util.h $(EFLDR)comp.h $(DFLDR)decomp.h
source/decoder/decomp.o: 	$(CFLDR)bitio.h $(CFLDR)util.h \
									$(CFLDR)header.h
source/encoder/comp.o: 	$(CFLDR)bitio.h $(EFLDR)hash_table.h \
								$(CFLDR)util.h $(CFLDR)header.h

.PHONY: clean
clean:
	-@rm -rf $(BUILDFLDR)
