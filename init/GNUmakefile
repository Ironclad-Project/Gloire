PREFIX = /
AS = as
LD = ld

.PHONY: all install clean

all: init

init:
	$(AS) main.S -o init.o
	$(LD) init.o -o init

install:
	install -d "$(DESTDIR)$(PREFIX)/sbin"
	install init "$(DESTDIR)$(PREFIX)/sbin"

clean:
	rm -rf init.o init
