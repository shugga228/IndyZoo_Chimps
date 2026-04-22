# IndyZoo_Chimps

## Hardware
- Adafruit Feather M0 Adalogger
- Adafruit Feather Sound board FX
- Speaker
- Arducam Mini OV2640
- Push button
- MicroSD card
- RTC

## Features
- Button-triggered image capture and audio playback 
- Saves JPEG images and diagnostic data to SD card 
- Uses SPI + I2C


## Wiring
CM = camera, AL = adalogger, SB = soundboard, RTC = real time clock

Buttons
1. Button 1 → one end pin 5 (AL), other end GND (AL)
2. Button 2 → one end pin 6 (AL), other end GND (AL)
3. Button 3 → one end pin 9 (AL), other end GND (AL)
4. Button 4 → one end pin 10 (AL), other end GND (AL)

RTC
1. GND (RTC) → GND (AL)
2. VCC (RTC) → 3V (AL)
3. SDA (RTC) → SDA (AL)
4. SCL (RTC) → SCL (AL)
Soundboard
1. Vin (SB) → USB (AL)
2. TX (SB) → RX (AL)
3. RX (SB) → TX (AL)
4. UG (SB) → GND (AL)
5. GND (SB) → GND (AL)
6. Audio output → speaker (basically a headphone jack)

Camera
1. MOSI (CM) → MOSI (AL)
2. MISO (CM) → MISO (AL)
3. SCK (CM) → SCK (AL)
4. SDA (CM) → SDA (AL)
5. SCL (CM) → SCL (AL)
6. VCC (CM) → 3V (AL)
7. CS (CM) → pin 12 (AL)
8. GND (CM) → GND (AL)

Note
Use power rails on the breadboard for 3V and GND to avoid running out of space.