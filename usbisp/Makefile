DESTDIR := /usr/local
CC := cc
CFLAGS := -O2

wchisptool:main.c
	$(CC) $(CFLAGS) main.c -o wchisptool `pkg-config --cflags --libs libusb-1.0`

clean:
	rm -f wchisptool

install: wchisptool
	cp wchisptool $(DESTDIR)/bin

