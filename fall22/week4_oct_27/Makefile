CC = clang
CFLAGS = -D_GNU_SOURCE -Weverything -std=c2x
BINS = racer read safe
.PHONY: all clean
all: $(BINS) files
files:
	rm -f fine flag
	echo "SECRET" > flag
	echo "fine to read" > fine
	sudo chown root:root flag read safe
	sudo chmod 04001 read safe #setuid binaries
	sudo chmod 0400 flag #only root can access this file
clean:
	rm -f $(BINS) fine flag
