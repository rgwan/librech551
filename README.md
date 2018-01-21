LibreCH551
----------

A libre open-source flasher program and SDK library for WCH(Nanjing QingHeng Corp.)'s USB microcontroller CH551.

The CH551 is a extremely low-cost 8051 USB microcontroller which about 1.5 CNY in retail. It is suitable for many low-end USB applications.

Currently working in progress, but all functions is working on CH551 except option bytes. I would like to support CH55x series microcontroller.

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
