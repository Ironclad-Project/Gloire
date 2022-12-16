ifeq ($(origin CC), default)
	CC := cc
endif
CFLAGS  ?= -O2 -pipe -g -Wall -Werror -Wextra
PREFIX  ?= /usr/local

# Targets and their rules.
override TARGET  := gcon
override CSOURCE := $(shell find ./ -type f -name '*.c')
override OBJ     := $(CSOURCE:.c=.o)

.PHONY: all clean install

all:
	$(MAKE) limine-terminal
	$(MAKE) $(TARGET)

limine-terminal:
	git clone https://github.com/limine-bootloader/terminal limine-terminal
	cd limine-terminal && git checkout 23024c038f792f41418f023d008867fc5a8b37f9

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -I. -Ilimine-terminal -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJ)

install:
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -s $(TARGET) "$(DESTDIR)$(PREFIX)/bin/"
