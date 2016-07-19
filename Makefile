PROGS += lz78

#CFLAGS += -Werror 
CFLAGS += -Wall -Wunused-function
CFLAGS += -Wextra
#CFLAGS += -DDO_STAT

SRCS = source/common/bitio.c source/common/hash_table.c 
SRCS += source/decoder/decomp.c
SRCS += source/prova.c
OBJS= $(SRCS:%.c=%.o)
CLEANFILES = $(PROGS) $(OBJS) 
BUILDFLDR = build/

all: $(PROGS)
	$(CC) $(CFLAGS) -o $@ $^

lz78: prova.o decomp.o

decomp.o: decomp.h bitio.h
prova.o: bitio.h hash_table.h decomp.h

.c.o:
	$(CC) -c $_

clean:
	-@rm -rf $(BUILDFLDR)
