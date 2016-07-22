PROGS = lz78

#CFLAGS += -Werror 
CFLAGS = -Wall -Wunused-function
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

lz78: source/lz78.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

source/lz78.o: source/encoder/comp.o source/decoder/decomp.o
source/decoder/decomp.o: 	source/common/bitio.h source/common/util.h \
									source/common/header.h
source/encoder/comp.o: 	source/common/bitio.h source/encoder/hash_table.h \
								source/common/util.h source/common/header.h

.PHONY: clean
clean:
	-@rm -rf $(BUILDFLDR)
