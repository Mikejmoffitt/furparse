APPNAME := furparse

CC := gcc
INSTALL_PREFIX := /usr/bin
ifdef SYSTEMROOT
	APPEXT := .exe
endif

LDFLAGS := -lz
CFLAGS := -O3 -Wall -std=c11

EXECNAME := $(APPNAME)$(APPEXT)

.PHONY: clean

all: $(EXECNAME)

$(EXECNAME): $(APPNAME).c
	$(CC) $< $(CFLAGS) $(LDFLAGS) -o $@

install: $(EXECNAME)
	cp $< $(INSTALL_PREFIX)/

clean:
	rm -f $(EXECNAME)
