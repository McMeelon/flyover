CC      ?= gcc
CFLAGS  ?= -O2 -Wall -Wextra
BIN     := bin/ad_scanner
SRC     := src/scanner.c

.PHONY: all clean run

all: $(BIN)

$(BIN): $(SRC)
	@mkdir -p bin
	$(CC) $(CFLAGS) -o $@ $<

run: $(BIN)
	./recon.sh

clean:
	rm -f $(BIN)
