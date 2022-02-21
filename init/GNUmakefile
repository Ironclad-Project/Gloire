PREFIX = /
CC = cc

.PHONY: all install clean

all: init

init:
	$(CC) main.c -o init

install:
	install -d "$(DESTDIR)$(PREFIX)/sbin"
	install init "$(DESTDIR)$(PREFIX)/sbin"

clean:
	rm -rf init
