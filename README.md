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

## OS Support
- Linux: works as intended out of the box
    - I do not know if all distros support high res scrolling but so far it has worked on all the distro's i have tried
- Windows: works as intended out of the box
- MacOS: does not natively support high res scrolling, I have had the best results using [Better Mouse](https://better-mouse.com/)


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

## Credits
The idea comes from [This Video](https://www.youtube.com/watch?v=FSy9G6bNuKA&t=287s) by
[Engineer Bo](https://www.youtube.com/@engineerbo) with some extra features of my own.  
Without that video a would never have had the idea to create a project like this on my own and
wouldn't even know that High res scrolling was a thing
There is absolutely no doubt in my mind that his version is significantly higher quality and more refined
than this project, this is more aimed for people that like to build things themselves.

The project is built on top of [Zephyr Project](https://www.zephyrproject.org/).

I spent a good amount of time reading through the bluetooth logic from [ZMK](https://zmk.dev/) to
get my bluetooth working, I don't recall directly copying any of their code but its likely to be
pretty damn similar in a lot of ways.

[Claude.ai](https://claude.ai) was my Clanker of choice for troubleshooting and helping with research
but was not used for code generation. It was of questionable usefulness throughout the project and
I am sceptical of if it actually saved me any time but I did make use of it to some degree and I
think that its important to recognise its usage.
