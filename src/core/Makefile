CC=gcc
CFLAGS=-I. -Wall

DEPS = cli.h

ROOTDIR = ../..
OBJDIR = obj
BINDIR = $(ROOTDIR)/bin
_OBJ = main.o cli.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

$(OBJDIR)/%.o: %.c $(DEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/archiplex: $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

run_cli: $(BINDIR)/archiplex
	./$(BINDIR)/archiplex

clean:
	rm -rf $(OBJDIR) $(BINDIR)
