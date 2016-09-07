PROGS = lz78

#CFLAGS += -Werror 
CFLAGS = -Wall -g -Wunused-function
CFLAGS += -Wextra
LDFLAGS = -lm # math.h is not a standard lib, we need to include it
LDFLAGS += -lcrypto #for md5 checksum 
#CFLAGS += -DDO_STAT

CFLDR = source/
BUILDFLDR = build/
SRCS = $(wildcard source/*.c)
OBJS = $(SRCS:%.c=%.o)
CLEANFILES = $(PROGS) $(OBJS)

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

all: $(PROGS)
	test -d $(BUILDFLDR) || mkdir $(BUILDFLDR)
	mv -t $(BUILDFLDR) $(CLEANFILES)

lz78: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

lz78.o:	$(addprefix $(CFLDR), util.h comp.h decomp.h)
decomp.o: 	$(addprefix $(CFLDR), bitio.h util.h header.h)
comp.o: 	$(addprefix $(CFLDR), bitio.h hash_table.h util.h header.h)

.PHONY: clean
clean:
	-@rm -rf $(BUILDFLDR)
