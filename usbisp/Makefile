DESTDIR := /usr/local
wchisptool:main.c
	cc main.c -o wchisptool `pkg-config --cflags --libs libusb-1.0`

clean:
	rm -f wchisptool

install:
	cp wchisptool $(DESTDIR)/bin

