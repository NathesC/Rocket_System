//Bristol SEDS Rocket system  - BASE STATION - Nathan Cubitt 2021
//       >>>>>>>>>>>>>  will receive and record data to miniSD card <<<<<<<<<<<<<<<<<

// uses Adafruit RFM69HCW transceiver, SD card reader,
// folllowing connections are for connecting sensors etc to Arduino Nano
//pins: SDCard 5v, GND, CLK -> D13, DO -> D12, DI ->D11, CS ->12
//pins: Radio 3.3v, GND, SCLK -> D13, MISO ->D12, MOSI -> D11, CS -> D4, RST -> D2, GO -> D3

//SD card will record raw data in following format:
// ident : time : GPS co-ords : max altitude : current altitude
// the colon will be used to split the data

#include <SPI.h>
#include <RH_RF69.h>
#include <SD.h>

/************ Radio Setup ***************/

// Change to 434.0 or other frequency, must match RX's freq!
#define RF69_FREQ 433.0

#define RFM69_INT     3  // 
#define RFM69_CS      4  //
#define RFM69_RST     2  // "A"
#define LED           13

// Singleton instance of the radio driver
RH_RF69 rf69(RFM69_CS, RFM69_INT);

//int16_t packetnum = 0;  // packet counter, we increment per xmission
float prevAlt;
File rawData;
int counter = 0; //used to determine initial altitude at start



void setup()
{
  //usually set rate to 115200 but 9600 may increase range
  Serial.begin(9600);
  while (!Serial) {
    delay(1);  // wait until serial console is open, remove if not tethered to computer
  }

  pinMode(LED, OUTPUT);
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, LOW);

  Serial.println(F("\nSEDs BASE STATION INIT"));

  // manual reset
  digitalWrite(RFM69_RST, HIGH);
  delay(10);
  digitalWrite(RFM69_RST, LOW);
  delay(10);

  if (!rf69.init()) {
    Serial.println(F("RFM69 radio init failed"));
    while (1);
  }
  Serial.println(F("RFM69 radio init OK!"));

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM (for low power module)
  // No encryption
  if (!rf69.setFrequency(RF69_FREQ)) {
    Serial.println(F("setFrequency failed"));
  }

  // If you are using a high power RF69 eg RFM69HW, you *must* set a Tx power with the
  // ishighpowermodule flag set like this:
  rf69.setTxPower(20, true);  // range from 14-20 for power, 2nd arg must be true for 69HCW

  // The encryption key has to be the same as the one in the server
  uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                    0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08
                  };
  rf69.setEncryptionKey(key);

  // init the SD card
  pinMode(10, OUTPUT);
  if (!SD.begin(10)) {
    Serial.println(F("SD CARD ERROR"));
    while (1);
  }

  pinMode(LED, OUTPUT);

  Serial.print(F("RFM69 radio @"));  Serial.print((int)RF69_FREQ);  Serial.println(F(" MHz"));

}


void loop() {

  int delim, delim1, delim2, delim3;
  
  if (rf69.available()) {
    // Should be a message for us now
    uint8_t buf[RH_RF69_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf69.recv(buf, &len)) {
      if (!len) return;
      buf[len] = 0;
      rawData = SD.open("Flight.txt", FILE_WRITE);
      Serial.print("\n\nReceived: ");
      String input = (char*)buf;
      
      // painful code to split the data on the colon
      delim = input.indexOf(":");
      delim1 = input.indexOf(":", delim + 1);
      delim2 = input.indexOf(":", delim1 + 1);
      delim3 = input.indexOf(":", delim2 + 1);

      String ident = input.substring(0 , delim);
      String time = input.substring(delim + 1, delim1);
      String loc = input.substring(delim1 + 1, delim2);
      String max = input.substring(delim2 + 1, delim3);
      String alt = input.substring(delim3 + 1);

      // set variables for displaying data
      float maxHeight = max.toFloat();
      float currAlt = alt.toFloat();
      float speed;
      float absSpeed; //correct to absolute value
      if (counter == 0) {  //set for initial reading
        speed = 0;
      }
      else {
        speed = prevAlt - currAlt;
      }
      counter = counter + 1;

      if (speed == 0.00) {  // stop division of zero
        absSpeed = 0;
      } else {
        absSpeed = fabs(speed / 2); //return absolute value of speed at m/s
      }

      //output data via serial output
      Serial.print(ident);
      Serial.print("\nTIME: " + time);
      Serial.print("\nLOCATION: " + loc);
      Serial.print("\nMAX ALT: " + max);
      Serial.print("\nCURRENT ALT: " + alt);
      Serial.print("\nSPEED: "); Serial.print(absSpeed); Serial.print(" m/s");
      prevAlt = currAlt; //change previous Altitude value for next loop

      //output raw data to SD card
      if (rawData) {
        rawData.println(input);
      } else {
        Serial.print(F("\nDATA NOT BEING RECORDED"));
      }
      rawData.close();


    } else {
      Serial.println("Receive failed");
    }
  }
}


void Blink(byte PIN, byte DELAY_MS, byte loops) {
  for (byte i = 0; i < loops; i++)  {
    digitalWrite(PIN, HIGH);
    delay(DELAY_MS);
    digitalWrite(PIN, LOW);
    delay(DELAY_MS);
  }
}
