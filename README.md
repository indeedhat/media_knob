# Media Knob
A little HID device to control media and scrolling.

## Controls
### Scroll mode
- spin for high def scrolling
- press and spin to zoom

### Media Mode
- spin to change volume
- double tap to play/pause
- press and spin to skip to next/prev track


## Roadmap
### Software
- [x] scroll mode
    - [x] high res scrolling
    - [x] zoom
- [x] media media
    - [x] play/pause
    - [x] volume
    - [x] next/prev
- [ ] mode switching (scroll/media)
    - [x] switch between scroll and media mode
    - [ ] save mode to eeprom so the state is restored on startup
- [ ] bluetooth connectivity
    - [ ] low power mode

### Hardware
- [x] connect as5600 sensor
- [x] trigger a button when you press on the dial
- [x] mode switching button
- [ ] sort out battery

### Cad
- [x] dial
- [x] bottom cover
- [x] main body (needs updating for buttons)

### Other
- [ ] create documentation
- [ ] upload final cad files

## Possible tasks
It might be nice to have a macro mode that lets you configure what the dial does in an application
running on the host device.

