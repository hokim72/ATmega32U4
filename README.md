## Test Board

#### [Arduino Leonardo](https://www.arduino.cc/en/Main/ArduinoBoardLeonardo)

#### Arduino Leonardo pinout

![Arduino Leonardo pinout](Ardunio_leonardo_pinout.jpg)

#### Bootloader

[BootloaderCDC.hex](https://github.com/hokim72/AVR-Common/blob/master/BootloaderCDC/BootloaderCDC.hex)

```
$ avrdude -p atmega32u4 -c usbasp -U flash:w:BootloaderCDC.hex
```

## Prerequisite

#### [AVR-Common](https://github.com/hokim72/AVR-Common)

## Compile and build

```
$ cd ~/work/avr
$ git clone https://github.com/hokim72/ATmega32U4.git
$ mkdir ATmega32U4/project_dir/build; cd ATmega32U4/project_dir/build
$ cmake-avr ..
$ make flash
```

