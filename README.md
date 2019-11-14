## What is VoodooSMBus? [![Actions Status](https://github.com/leo-labs/VoodooSMBus/workflows/CI/badge.svg)](https://github.com/leo-labs/VoodooSMBus/actions)

VoodooSMBus solves two related problems: 
  
  1. VoodooSMBus is a project that provides a kernel extension (driver) for macOS that adds support for the SMBus capabilities of Intel I/O Controller Hubs (ICH), also called i801 SMBus. 
  2. VoodooSMBus also contains a slave device driver for the ELAN Touchpad device found on the SMBus of some Thinkpad models (T480s, L380, P52) making it possible to use multitouch gestures efficiently.

This project is heavily inspired by https://github.com/alexandred/VoodooI2C and a complete reimplementation of the ideas in https://github.com/gokula/ELANSMBus. Multitouch support is made possible by the implementation VoodooI2C.

VoodooSMBus ports the `i2c-i801`<sup>[1](#i2c-i801)</sup> driver and the `elan_i2c_smbus`<sup>[2](#elan_i2c_smbus)</sup> from linux to macOS.

In the future we might want to split up the project,  similar as it has been done in VoodooI2C, into a controller kext and kexts for the slave device drivers.

## Installation

- Add `VoodooPS2Controller.kext`
- Add the patch in `config.plist.patch`, so `VoodooPS2Controller` does not attach itself to the PS2 interface of the touchpad
- Delete `VoodooPS2Controller.kext/Contents/PlugIns/VoodooPS2Mouse.kext`, otherwise the touchpad will not work after sleep
- Add the four patches found here https://github.com/leo-labs/macOS-ThinkPad-T480s/blob/master/EFI/CLOVER/config.example.plist#L436-L467 and https://github.com/leo-labs/macOS-ThinkPad-T480s/blob/master/EFI/CLOVER/config.example.plist#L500-L535 to supress loading of those kexts
- Install this kext

## Configuration

Some settings can be configured in the `Configuration` dictionary in `Info.plist`:

* `DisableWhileTyping` Enable disable while typing functionality. After a key press the touchpad is deactivated for a certain amount of time, specified by
* `DisableWhileTypingTimeoutMs` The amount of time in milliseconds that touch input is ignored after typing
* `DisableWhileTrackpoint` Disables the touchpad when the trackpoint is in use.
* `DisableWhileTrackpointTimeoutMs` The amount of time in milliseconds that touch input is ignored after trackpoint usage
* `IgnoreSetTouchpadStatus` Ignores messages from the keyboard driver to disable the touchpad. If not ignored, the touchpad can usually be toggled with the `PrtSc` key. 

## Current Status

Currently the following Intel I/O Controller Hubs are supported and tested:

| Name                   | Id             |  Device              |
| ---------------------- | -------------- | -------------------- |
| Sunrise Point-LP (PCH) | `pci8086,9d23` | Thinkpad T480s, L380 |
| Cannon Lake-H (PCH)    | `pci8086,a323` | Thinkpad P52         |


It should be trivial to add support for all controllers listed in <sup>[1](#i2c-i801)</sup>. 

Trackpoint support is implemented, make sure to activate the trackpoint in BIOS.

For a list of planned features, see https://github.com/leo-labs/VoodooSMBus/labels/enhancement

## Supported Gestures

For supported gestures please see https://voodooi2c.github.io/#Supported%20Gestures/Supported%20Gestures


## Acknowledgements

Thanks to:
- @gokula for the first version of an SMBus driver for the touchpad https://github.com/gokula/ELANSMBus
- @alexandred, @coolstar, @kprinssu (& others) for the development of the VoodooI2C kernel extension, VoodooI2CELAN satellite and the native gestures implementation. https://github.com/alexandred/VoodooI2C & https://github.com/kprinssu/VoodooI2CELAN/tree/master
- @rehabman for the VoodooPS2 kernel extension and all other stuff he makes. https://github.com/RehabMan/OS-X-Voodoo-PS2-Controller
- @dukzcry for his Intel ICH SMBus controller which was of great help in the driver development. https://github.com/dukzcry/osx-goodies/tree/master/ic 
- all linux kernel driver developers (especially the ones responsible for the i2c/smbus implementation and the elan i2c/smbus driver)

## Footnotes

<a name="i2c-i801">1</a>: https://github.com/torvalds/linux/blob/master/drivers/i2c/busses/i2c-i801.c

<a name="elan_i2c_smbus">2</a>: https://github.com/torvalds/linux/blob/master/drivers/input/mouse/elan_i2c_smbus.c
