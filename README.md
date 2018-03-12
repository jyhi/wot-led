# Arduino LED with Web Things API

This is a simple & small implementation of Mozilla's [Web Thing API], which let
you control the LED connected on Arduino pin 8 through the REST API (which means
you can also control the LED using the Mozilla Web Thing Gateway).

This implementation makes use of a SD card to store textual contents (e.g. JSON,
HTML).

To run this program, you need:

- Arduino
- Arduino Ethernet Shield (with W5100)
- LED attached on pin 8

This implementation make use of [ArduinoJson] to support JSON parsing. Currently
it can run on an Arduino UNO with:

- Full serial debug message, but no mDNS or DHCP support
- DHCP support, but no mDNS or full serial debug message

... since the Arduino UNO has only 32K flash and 2K SRAM, limiting the sketch
size. (the two configuration above both occupies 30K+ ROM)

[Web Thing API]: https://iot.mozilla.org/wot
[ArduinoJson]:   https://arduinojson.org

## Build

Use [PlatformIO], run:

```bash
# `pio` is the shorthand of `platformio`
pio run            # Build
pio run -t upload  # Burn
pio run -t monitor # Attach serial terminal
```

Arduino IDE is not supported.

[PlatformIO]: https://platformio.org/

## Configuration

You can configure to let this program use mDNS or DHCP or not. Please have a
look of [platformio.ini](platformio.ini).

## TODO

I write this program for fun.

- [ ] Add ESP8266 support

## License

This program is licensed under [The MIT License](COPYING).
