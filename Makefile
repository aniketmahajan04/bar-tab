# Compiler and Flags
all:
	@mkdir -p bin
	gcc src/main.c -o bin/bartab -Wall `pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0`

clean:
	rm -f bin/bartab

.PHONY: all clean
