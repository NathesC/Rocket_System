//Bristol SEDS Rocket system  - GROUND TEST - Nathan Cubitt 2021
//       >>>>>>>>>>>>>  will transmit gps to base station <<<<<<<<<<<<<<<<<
//       >>>>>>>>>>>>>  will flash LED if everything works          <<<<<<<<<<<<<<<<<



// With a NANO use Adafruit_GPS.h version 1.1.3 or earlier
//as per issue 118  https://github.com/adafruit/Adafruit_GPS/issues/118

// THIS VERSION IS FOR GROUND TESTING WITH A LAPTOP - IT WILL SAVE TO SDCARD
// AND GIVE A SERIAL READING


// uses Adafruit RFM69HCW transceiver, STEMMA mini GPS,
// folllowing connections are for connecting sensors etc to Arduino Nano
//pins: GPS 3.3v, GND, RX1 -> D7, TX0 -> D8
//pins: Radio 3.3v, GND, SCLK -> D13, MISO ->D12, MOSI -> D11, CS -> D4, RST -> D2, GO -> D3
//pins: LED status light GND, D9(+ resistor)



#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <SPI.h>
#include <RH_RF69.h>


SoftwareSerial mySerial(8, 7);
Adafruit_GPS GPS(&mySerial);

#define RF69_FREQ 433.0 //change to match the frequency you are using
#define RFM69_INT     3  // 
#define RFM69_CS      4  //
#define RFM69_RST     2  // "A"
#define LED           13
// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

// SET RANGE TO TRUE FOR BAUD 9600, FALSE FOR 115200
// SET FLIGHT TO TRUE FOR A LAUNCH, FALSE FOT TEST
// NB FLIGHT = TRUE WILL DISABLE SERIAL OUTPUT TO LAPTOP

boolean range = true; 
int rate = 115200;
boolean flight = false; //set to true for launch, false for testing
float maxHeight;
int faultFlag = 0; //be used to indicate faults with Serial off
int bootLight = 5; //LED will flash to show it is booting up

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO  false


//*****************SET UP *****************

void setup()
{
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // trying 9600 to 
  if (range == true){
    rate = 9600;
  }
  
  if (flight == false) { //will only run if connected to computer by USB
    Serial.begin(rate);
    while (!Serial); // waits for serial to be ready
    Serial.print(F("\nSEDS ROCKET TEST - set FLIGHT TO TRUE BEFORE LAUNCH"));
  }

  // ***** GPS SETUP*****
  // default for this GPS is 115200
  GPS.begin(rate);
  // turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // Set the update rate as recommended
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  delay(1000);
  // Ask for firmware version
  mySerial.println(PMTK_Q_RELEASE);

  
  // ***** RADIO SETUP*****
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);
  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    faultFlag = 2;
    while (1);
  }
  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    faultFlag = 3;
  }
  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);
  
  pinMode(5, OUTPUT);   //sets Pin for LED
  delay(1000);
  if(flight == false){
    Serial.println(F("\ninits successful"));
  }
  
}

uint32_t timer = millis();




void loop()
{
 //LED bootLight will flash 5 times to show it is working
  while (bootLight > 0) {
    digitalWrite(9, HIGH);         // sets LED on
    delay(1000);
    digitalWrite( 9, LOW);         // LED off
    delay(1000);
    bootLight--;
  }
  
  char c = GPS.read();
  if ((c) && (GPSECHO))
    Serial.write(c);
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  if (millis() - timer > 2000) {
    timer = millis(); // reset the timer
    Serial.print("\nSatellites Detected: "); Serial.print(GPS.satellites);
    if (GPS.fix) { 
      if (maxHeight < GPS.altitude) {
        maxHeight = GPS.altitude;
      }

      //radio message - using chars to minimise String usage
      char radiopacket[60]; //used for radio
      char split = ':'; //used to split message at receiver
      char timeSplit = '.'; //for ease of human reading
      char cordSplit = ','; //for ease of human reading
      float coordLat;
      coordLat = GPS.latitude, 4 + GPS.lat, 4;
      float coordLong;
      coordLong = GPS.longitude, 4 + GPS.lon, 4;
      String tx = (F("SEDS_TX: "));

      //assemble the message
      tx.concat(GPS.hour);
      tx.concat(timeSplit);
      if (GPS.minute < 10) {
        tx.concat('0' + (GPS.minute));
      } else {
        tx.concat(GPS.minute);
      }
      tx.concat(timeSplit);
      if (GPS.seconds < 10) {
        tx.concat('0' + (GPS.seconds));
      } else {
        tx.concat(GPS.seconds);
      }
      tx.concat(F("(GMT): "));
      tx.concat(coordLat);
      tx.concat(cordSplit);
      tx.concat(coordLong);
      tx.concat(split);
      tx.concat(maxHeight);
      tx.concat(split);
      tx.concat(GPS.altitude);

      tx.toCharArray(radiopacket, 60);
      //itoa(packetnum++, radiopacket + 13, 10);


      // Send a message!
      rf69.send((uint8_t *)radiopacket, strlen(radiopacket));
      rf69.waitPacketSent();


    } else {
      char los[40] = "No Satellite Signal Received";
      //itoa(packetnum++, los + 13, 10);
      rf69.send((uint8_t *)los, strlen(los));
      rf69.waitPacketSent();
    }
  }
  if(flight == false){
  if (faultFlag == 2) {
    Serial.println(F("Component Error: Radio init"));
  }
  if (faultFlag == 3) {
    Serial.println(F("Component Error: Radio Freq"));
  }
  }
  if (faultFlag >0){
    digitalWrite(9, HIGH); 
  }
}
