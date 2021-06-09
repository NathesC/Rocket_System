# Rocket_System
Arduino based rocket telemetry system

<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/seds.jpg">
</p>

## Arduino Nano GPS data system built for Bristol SEDS competition 2021

The system is split between a transmitting rocket system and a receiving base station.
### Parts
- Arduino Nano x 2
- Adafruit Stemma GPS
- Adafruit RFM69HCW x 2
- Adafruit micro SD card shield
- LED, resistor

### Initial Plan
The initial plan was to have the rocket transmit and save data to an internal SD card.  Unfortunately the Nano
doesn't have enough memory to run GPS, radio and SD Card (see issue [118](https://github.com/adafruit/Adafruit_GPS/issues/118)) even when using Adafruit_GPS.h library version 1.1.3 or earlier.
**note version 1.1.3 has been used with this code, and that led to using Software Serial connections to be used**

The received transmissions would be piped into a dashboard display system most likely built in Processing.
<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/tx.jpg">
</p>
<p align="center">
  The Rocket System
  </p>

### Revised Plan

The rocket system transmits a line of data back, that will be split using a colon (:) as a delimiter.  It has an external LED to display system status before a launch - and has a boolean flight variable to allow for serial output testing with a laptop prior to flight. **Flight must be set to true for a launch**
It also has a boolean range that sets baud rate to 9600 to increase radio range, and can be false to go to default 115200.
<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/snippet.JPG">
</p>
<p align="center">
  Set boolean Range to change Baud Rate, set boolean flight to change from testing
  </p>


It transmits a header to identify it as a SEDs rocket, Time, GPS location, Max height reached, and Current height.
With **flight == false** the serial output will include the number of satellites being detected as without satellites there is no GPS data.  
The Stemma GPS is more accurate outside than inside, so expect strange data when testing!


The receiver displays text data include rate of climb/descent on serial output, and saves raw data to SD Card.  At a later date a dashboard system will be created, but for test flights this will work fine.

**note it is highly likely that the rocket will go out of range and then come back in on descent- a LoRa radio may be better in these circumstances**

<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/rx.jpg">
</p>
<p align="center">
  The Receiving Station
  </p>


### Future Changes

Ideally the Nanos would be replaced with more powerful systems (an earlier project had used BMP180to record altitudes, but the competition requirements required a GPS). The Stemma GPS can record data itself, and we may look in to that.

The code uses a few too many Strings and wil lbe changed into c-strings(char arrays).  However for test flight purposes this should be fine


### Wiring - Rocket System

Antenna: 16.5cm wire(for 433Mhz)  
GPS: 3.3v, GND, RX1 -> D7, TX0 -> D8  
Radio: 3.3v or 5v, GND, SCLK -> D13, MISO ->D12, MOSI -> D11, CS -> D4, RST -> D2, GO -> D3  
LED status light; GND, D9(+ resistor)

**note in diagram the Radio is going to 5v but 3.3v may be better for energy consumption**


<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/Rocket_wiring.jpg">
</p>
<p align="center">
  Rocket System - Wiring
  </p>


### Wiring - Base Station

Antenna: 16.5cm wire(for 433Mhz)  
SDCard: 5v, GND, CLK -> D13, DO -> D12, DI ->D11, CS ->12  
Radio: 3.3v, GND, SCLK -> D13, MISO ->D12, MOSI -> D11, CS -> D4, RST -> D2, GO -> D3  

<p align="center">
  <img  src="https://github.com/NathesC/Rocket_System/blob/main/Media/base_station_wiring.jpg">
</p>
<p align="center">
  Base Station - Wiring
  </p>

### To Come
 feedback from the launch
