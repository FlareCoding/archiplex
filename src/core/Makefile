CC=gcc
CFLAGS=-I. -Wall

TARGET = archiplex

DEPS = cli.h

ROOTDIR = ../..
OBJDIR = obj
BINDIR = $(ROOTDIR)/bin
_OBJ = main.o cli.o
OBJ = $(patsubst %,$(OBJDIR)/%,$(_OBJ))

# Install directories
INSTALL_DIR 		:= /usr/local
INSTALL_BIN_DIR		:= $(INSTALL_DIR)/bin
INSTALL_SHARE_DIR 	:= $(INSTALL_DIR)/share/$(TARGET)

$(OBJDIR)/%.o: %.c $(DEPS)
	mkdir -p $(OBJDIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(BINDIR)/$(TARGET): $(OBJ)
	mkdir -p $(BINDIR)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

run_cli: $(BINDIR)/$(TARGET)
	./$(BINDIR)/$(TARGET)

install:
	@if [ $$(id -u) -ne 0 ]; then \
		echo "Please run 'make install' as root or with sudo"; \
		exit 1; \
	fi

	install -d $(INSTALL_BIN_DIR)
	install -m 755 $(BINDIR)/$(TARGET) $(INSTALL_BIN_DIR)
	install -d $(INSTALL_SHARE_DIR)/tools
	install -d $(INSTALL_SHARE_DIR)/codegen
	cp -r $(ROOTDIR)/tools $(INSTALL_SHARE_DIR)
	cp -r $(ROOTDIR)/src/codegen $(INSTALL_SHARE_DIR)

uninstall:
	@if [ $$(id -u) -ne 0 ]; then \
		echo "Please run 'make install' as root or with sudo"; \
		exit 1; \
	fi

	rm -rf $(INSTALL_BIN_DIR)/$(TARGET)
	rm -rf $(INSTALL_SHARE_DIR)

clean:
	rm -rf $(OBJDIR) $(BINDIR)
