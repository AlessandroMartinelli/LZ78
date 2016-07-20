PROGS += lz78

#CFLAGS += -Werror 
CFLAGS += -Wall -Wunused-function
CFLAGS += -Wextra
CFLAGS += -lm # math.h is not a standard lib, we need to include it
#CFLAGS += -DDO_STAT

SRCS = source/common/bitio.c
SRCS += source/encoder/hash_table.c source/encoder/comp.c
SRCS += source/decoder/decomp.c
OBJS= $(SRCS:%.c=%.o)
CLEANFILES = $(PROGS) $(OBJS) 
BUILDFLDR = build/

.c.o:
	$(CC) -c $_
	
all: $(PROGS)
	$(CC) $(CFLAGS) -o $@ $^

lz78: comp.o decomp.o

decomp.o: bitio.h util.h
comp.o: bitio.h hash_table.h util.h

clean:
	-@rm -rf $(BUILDFLDR)
