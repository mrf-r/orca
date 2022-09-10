### orca mini keyboard controller high resolution firmware (MIDI 1.0, with 14bit CC)

The are no high resolution midi devices on market, so i bought a cheap one to make my own. Worlde ORCA mini 25 seems like the perfect keyboard for this - 8 pots, display for standalone adjustments, rgb pads. CPU is NUC123 48pins with small memory (multitrack sequencing are unlikely), 10bit ADC (low noise, ok), M0 (no hw divisions). Pads connected to ADC, but HW is extremely non-linear and even the light touches generate max values - velocity/aftertouch makes no sense. RGB leds are some kind of WS2812 - too bright and too power hungry to use full scale brightness. No EEPROM. Internal space are enough for any additional board (ESP32/Daisy/Teensy/..), can be connected through OLED's IIC bus in multimaster mode (NO DMA).

project optimized for WSL(gcc,make,gdb) + Windows(openocd). script contain absolute paths!

project in early alpha atm.

goals:
 - basic usb/uart midi io, high throughput
 - midi.c ???
 - controller mode (test mode)
 - knobs, "wheels"
 - keys, velocity, damper?
 - pads
 - arp, latching
 - simple sequencing?
 - menu system
 - midi effects
 - iic multimaster?
 - bootloader with usb midi (arturia's protocol?)

### git versioning
 - use `git flow`
 - when release, use tags "%d.%d.%d" with annotations, example: `git flow release start 1.1.3`

### Debug
 - gdb with python support (if you want low level registers view)
 - `pip install -U ` `cmsis-svd` `terminaltables`. https://github.com/1udo6arre/svd-tools
 - run `start_debug.sh`
 - uses Segger's RTT on port 5678 (WSL IP)
