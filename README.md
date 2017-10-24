# tomu-crc16

When debugging firmware you want to make sure that what you flahed onto a device
is what you intended to.

The TOMU bootloader has a command to get the CRC of the user flash memory.

This program calculates the same CRC from the file you're about to flash.

To Build:

  make crc16

To use:
  crc16 file.bin

To check: use the `c' command on the TOMU bootloader. 
