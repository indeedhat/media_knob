# Media Knob
A little HID device to control media and scrolling.

Tested on linux, macos and windows


![media knob](assets/media-knob.jpg)

## Build Guide
[Build Guide found heer](build-guide.md)

## Controls
The device has 3 inputs:
- Spin dial
- Side button (mode switch)
- Button in base under the foot opposite the USB port (Mod)

### Scroll mode

| Direction | Mod | Action |
| --- | --- | --- |
| Clockwise | No | Scroll Up |
| Counter Clockwise | No | Scroll Down |
| Clockwise | Yes | Zoom in |
| Counter Clockwise | Yes | Zoom out |

### Media Mode

| Direction | Mod | Action |
| --- | --- | --- |
| Clockwise | No | Volume Up |
| Counter Clockwise | No | Volume Down |
| Clockwise | Yes | Next track |
| Counter Clockwise | Yes | Previous track |
| N/A | Double Tap | Play/Pause |


## Bluetooth Operation
- The device shows up as "Knoblet".
- It will allow up to 5 devices to be paired but only one device to be connected at a time.
- pairing to a 6th device will remove the oldest pairing
- when pairing to a device that it already has a pairing for but the key has changed (for example if you remove the device from your pc and then re add it) connection will fail the first time and the pairing will be cleared for that device. The secrond time you try connect it will pair correctly. This is intended functionality to remove the need for a dedicated clear pairing button.

## Roadmap
### Software
- [x] scroll mode
    - [x] high res scrolling
    - [x] zoom
- [x] media media
    - [x] play/pause
    - [x] volume
    - [x] next/prev
- [x] mode switching (scroll/media)
- [ ] bluetooth
    - [x] Connect via bluetooth
    - [ ] low power mode
        - in progress
    - [x] properly report battery life
- [ ] usb
    - [x] get usb working alone
    - [ ] get usb working along side bluetooth

### Hardware
- [x] connect as5600 sensor
- [x] trigger a button when you press on the dial
- [x] mode switching button
- [x] sort out battery

### Cad
- [x] dial
- [x] bottom cover
- [x] main body (needs updating for buttons)

### Other
- [x] create build guide
    - needs updating
- [x] upload STL files for 3d printing

## Possible tasks
It might be nice to have a macro mode that lets you configure what the dial does in an application
running on the host device.


## Wiring
### as5600
- scl -> 011 - yellow
- sda -> 100 - green

### input
- mode -> 111 - blue
- mod  -> 010 - white
