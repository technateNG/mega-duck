# mega-duck
This a prototype of cheap USB device to perform "Bad-USB attack". 

## What's a Bad-USB attack?
It's a kind of physical attack in which attacker inserts USB device impersonating keyboard to target machine.  
After insertion, device executes payload within few seconds. 

## What's the mega-duck?
It's a prototype of 8-bit AVR based device with cost of all pieces less then 4$.  
Mega-duck has two main modes of operation - Weaponized and Loader. User can switch modes by pressing button on device.  
When mega-duck is in weaponized mode, then after plug in to USB port, it introduces itself as keyboard and execute its payload.  
In Loader mode, after plug in user can load into device.  
Payload for the device is stored in EEPROM memory inside microcontroller - in ATMega8A it's 512 bytes.  
Device was tested and it's working on Windows 10 and Linux.

## What's inside repository?
Mega-duck has two main components.
Firmware - contains program which needs to be loaded inside microcontroller excutable flash memory.  
Connector - this is standalone x86/x86 console program which detects pluged in device on Loader mode. 
Through connector user can inserts his payload inside device.  
Schema - KiCad files and pdf schema.

## Which microcontroller is used here?
Microcontroller used in this desgin is ATMega8A with external crystal.
