PROGS += lz78

#CFLAGS += -Werror 
CFLAGS += -Wall -Wunused-function
CFLAGS += -Wextra
LDFLAGS = -lm # math.h is not a standard lib, we need to include it
LDFLAGS += -lcrypto #for md5 checksum 
#CFLAGS += -DDO_STAT

SRCS = source/common/bitio.c source/common/header.c
SRCS += source/encoder/hash_table.c source/encoder/comp.c
SRCS += source/decoder/decomp.c
OBJS= $(SRCS:%.c=%.o)
CLEANFILES = $(PROGS) $(OBJS) 
BUILDFLDR = build/
	
all: $(PROGS)
	test -d $(BUILDFLDR) || mkdir $(BUILDFLDR)
	mv -t $(BUILDFLDR) $(CLEANFILES) 

lz78: source/encoder/comp.o source/decoder/decomp.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

source/decoder/decomp.o: source/common/bitio.h source/common/util.h source/common/header.h
source/encoder/comp.o: source/common/bitio.h source/encoder/hash_table.h source/common/util.h source/common/header.h

clean:
	-@rm -rf $(BUILDFLDR)
