INCDIR=inc
CC=gcc
CFLAGS=-I$(INCDIR) -std=c99 -ggdb -O2 -pg

OBJDIR=obj

_DEPS = bignum.h tests.h
DEPS = $(patsubst %,$(INCDIR)/%,$(_DEPS))

_OBJ = bignum.o tests.o 82k.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	@mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

82k: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

t: 82k
	./82k -t

.PHONY: clean t

clean:
	rm $(OBJDIR)/*.o
	rm 82k
