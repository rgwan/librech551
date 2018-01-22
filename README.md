LibreCH551
----------

A libre open-source flasher program and SDK library for WCH(Nanjing QinHeng Corp.)'s USB microcontroller CH55x.

The CH55x series microcontroller is extremely low-cost 8051 USB microcontroller series, the cheapest one, CH551, which about 1.5 CNY in retail. It is suitable for many low-end USB applications.

Currently we have tested out CH551 and CH554 device, all functions works fine except option byte.

If someone want to help me, please fork, and support more chips, Thanks!

Build & Usage
----------

Build: Type 'make' in "usbisp" directory.

Install:Type 'make install' in 'usbisp' directory, you can also pass DESTDIR to the script to describe installation path. 

In Linux, there's no driver required.

In Windows, you may have to use Zadig to install WinUSB driver for this device.

License
----------

GPLv3

Author(s)
----------

Zhiyuan Wan <h@iloli.bid>
