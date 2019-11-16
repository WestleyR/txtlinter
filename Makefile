
CC = gcc
CFLAGS = -Wall
TARGET = txtlinter

PREFIX = /usr/local

MAIN = src/main-txtlinter.c

COMMIT = "$(shell git log -1 --oneline --no-color || echo "ERROR: unable to get commit hash" >&2 ; echo "unknown")"

.PHONY:
all: $(TARGET)

.PHONY:
$(TARGET): $(MAIN)
	$(CC) $(CFLAGS) -DCOMMIT_HASH=\"$(COMMIT)\" -o $(TARGET) $(MAIN)

.PHONY:
static: $(MAIN)
	$(CC) $(CFLAGS) -DCOMMIT_HASH=\"$(COMMIT)\" --static -o $(TARGET) $(MAIN)

.PHONY:
install: $(TARGET)
	mkdir -p $(PREFIX)/bin
	cp -f $(TARGET) $(PREFIX)/bin

.PHONY:
clean:
	 rm -f $(TARGET)

.PHONY:
uninstall: $(PREFIX)/$(TARGET)
	rm -f $(PREFIX)/$(TARGET)

