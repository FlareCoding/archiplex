.PHONY: build clean install uninstall

all: build

build:
	$(MAKE) -C src/core

clean:
	$(MAKE) -C src/core clean

install:
	$(MAKE) -C src/core install

uninstall:
	$(MAKE) -C src/core uninstall