# CH559 USB

Code for the CH559 chip from WCH to use as a USB host, you can connect a mouse or keyboard to it and it will output a simple protocol via serial port 1.

you can edit the files and compile it via the compile.bat on windows, sdcc is included, so no extra tools needed.

the chflasher.exe can be used to upload a bin firmware to ch55x chips, it will be uploaded directly after compile, no WCH tool needed anymore.
Please install the libusb-win32 driver with the zadig tool ( https://zadig.akeo.ie/ ) to enable access to usb for the tool.

Including Arduino Example code to either output the data to serial or onto an st7789 display.

