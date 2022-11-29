ifeq ($(origin CC), default)
	CC := cc
endif
CFLAGS  ?= -O2 -pipe -g
PREFIX  ?= /usr/local

# Targets and their rules.
override TARGET  := gwmbootstrap
override CSOURCE := $(shell find ./ -type f -name '*.c')
override OBJ     := $(CSOURCE:.c=.o)

.PHONY: all clean install

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -I. -c $< -o $@

clean:
	rm -rf $(TARGET) $(OBJ)

install:
	install -d "$(DESTDIR)$(PREFIX)/bin"
	install -s $(TARGET)  "$(DESTDIR)$(PREFIX)/bin/"
