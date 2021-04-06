**Densha de Go! Nintendo Switch Controller**

Playing Densha de Go!! Hashirou Yamanote Sen (電車でGO!! はしろう山手線) for Nintendo Switch with 3D printed parts and Arduino.

# Firmware

## Arduino as Nintendo Switch Controller

The firmware folders contains the Arduino code to emulate the Pokken Tournament Pro Pad Wii U Controller. The code is designed to be run on AVR chip with native USB such as Arduino Micro or Aruduino Leonardo.

It uses Arduino Pluggable USB interface instead of LUFA, hence buildable right in Arduino. Also no need to tweak `boards.txt` for VID/PID since it's done in the code.

The code could also be reused for other controller project.

I have referred to the following projects but the code in this project is written from the ground up.

* https://github.com/kidGodzilla/woff-grinder
* https://github.com/shinyquagsire23/Switch-Fightstick
* https://github.com/bertrandom/snowball-thrower/blob/master/Joystick.c
* https://github.com/Jas2o/Leonardo-Switch-Controller/blob/master/libraries/Joystick/Joystick.cpp

## Usage

A potential meter/variable resistor (such as WH148) for the master controller/マスコン, connected to an analog pin. With 8 buttons for D-Pad and A/B/X/Y connected to digital IO pins.

Special keys/functions with the following chord inputs:

* D-Pad Left + Right: for L + R in controller detection
* D-Pad Top + Left: L
* D-Pad Top + Right: R
* D-Pad Top + Bottom: Home
* D-Pad Top + Left + Right: Plus
* D-Pad Bottom + Left: Save Snapshot
* D-Pad Bottom + Right: Save Video
* D-Pad all fours button: Reset the potential meter range saved in EEPROM
* D-Pad Top + Button Y: For driving 103 Series.
* D-Pad Top + Button B: For driving all other newer EMU. (Default when power on)

# Mechanical Design

TBD


# Demo

https://youtu.be/4daXNpcmuk0
