Build Executables
Generate EXE file for Microsoft Windows requires Microsoft Visual Studio 2017 and Windows Kits 10.0.15063.0 or above. 
If your system does not meet the stated requirement, go to Microsoft's website and download the installer for Microsoft Visual Studio 2017 Community Version (for free).
Note: It is possible to compile the source with legacy Visual Studio although it is not officially supported.
1.Download the entire repository and unzip to anywhere you like
2.Navigate to "msvc"  folder and create a folder called "libusb" under "msvc" directory
3.Go to https://github.com/libusb/libusb/releases to get the latest stable binary snapshots of libusb(e.g. libusb v1.0.21), 
download the file with extension "7z"  or "tar.bz" , then unzip it to "msvc\libusb" . Now your "msvc\libusb"  should contain at least these folders : "MS32", "MS64" and "include" 
4.Launch "msvc\librech551.sln" and choose your targeting platform (e.g. Release x64), then start compiling.
