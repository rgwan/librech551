LibreCH551
----------

A libre open-source flasher program and SDK library for WCH(Nanjing QinHeng Corp.)'s USB microcontroller CH55x.

The CH55x is a series of 8051 microcontroller with dedicated USB peripheral. CH553/CH554 can be programmed as a USB host.
The cheapest one, CH551 only costs 1.5 CNY in retail. It is ideal for many low-end USB applications.

Up to now, we have tested out CH551 and CH554, all functions work fine except option byte.

If someone want to help me, please __fork__ and support more chips, Thanks!

For Linux Users
----------

Build: Type `make` in `usbisp` directory.

Install:Type `make install` in `usbisp` directory, you can also pass DESTDIR to the script to describe installation path. 

In Linux, there's no driver required.


For Windows Users
----------
* Compiling in Windows requires __Microsoft Visual Studio 2017__ and __Windows Kits 10.0.15063.0 or above__. To build executables, first
download the entire repository and unzip to anywhere you like, then navigate to `msvc` folder and launch `librech551.sln` to start the build. 

__Driver installation__
1. Connect your CH55x to one of USB ports on your PC, make sure the MCU enters ISP mode.
2. Launch `tools\zadig-2.3.exe`, open `Options` menu and click `List All Devices`, 
you should be able to find your CH55x (Usually `USB Module`), optionally you can change its name by check the `Edit` box. 
3. Double check that the USB ID is correct, USB ID is VID plus PID, for CH554, the USB ID is `4348 55E0`.
4. Select `libusbK` and click `Install Driver`, quit Zadig after driver installation.

Command Line Parameters
------------
* __-f \<filename\>__ Erase the code flash and download binary codes to CH55x, note that this tool only accepts `.bin` files, `.hex` files cannot be used directly. 
To convert a hex file to bin file, on Linux, use `objdump`, on Windows, use `hex2bin.exe` provided in `tools`.
* __-g__ Execute the user program after downloading
* __-e__ Erase chip only
* __-D \<filename\>__ Read data flash and save to a file
* __-d \<filename\>__ Write the content of a file to data flash

License
----------

GPLv3

Author(s)
----------

Zhiyuan Wan <h@iloli.bid>
