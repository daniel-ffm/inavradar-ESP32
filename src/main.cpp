#include <lib/MSP.h>
#include <lib/LoRa.h>
#include <SSD1306.h>
#include <Arduino.h>
#include <esp_system.h>

#define SCK 5 // GPIO5 - SX1278's SCK
#define MISO 19 // GPIO19 - SX1278's MISO
#define MOSI 27 // GPIO27 - SX1278's MOSI
#define SS 18 // GPIO18 - SX1278's CS
#define RST 14 // GPIO14 - SX1278's RESET
#define DI0 26 // GPIO26 - SX1278's IRQ (interrupt request)
#define BAND 868E6 // 915E6

SSD1306 display (0x3c, 4, 15);

#define rxPin 17
#define txPin 23

MSP msp;

bool loraRX = 0; // new packet flag
String loraMsg;
byte loraAddress = 0x01; // our uniq loraAddress

// ----------------------------------------------------------------------------- String seperator split
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
// ----------------------------------------------------------------------------- LoRa
void sendMessage(String outgoing) {
  while (!LoRa.beginPacket()) {
  }
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket(false);                 // finish packet and send it
}

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  String incoming = "";                 // payload of packet
  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }
  if (!loraRX && getValue(incoming,',',0) == "ADS-RC") {
    //for (int i = 0; i < 8; i++) { }
    loraMsg = getValue(incoming,',',1);
    loraRX = 1;
  }
}

void initLora() {
	display.drawString (0, 16, "LORA");
  display.display();
  SPI.begin(5, 19, 27, 18);
  LoRa.setPins(SS,RST,DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    display.drawString (94, 16, "FAIL");
    while (1);
  }
  LoRa.sleep();
  LoRa.setSignalBandwidth(250000);
  LoRa.setCodingRate4(6);
  LoRa.setSpreadingFactor(7);
  LoRa.idle();
  LoRa.onReceive(onReceive);
  LoRa.enableCrc();
  LoRa.receive();
  Serial.print("- OK");
  display.drawString (100, 16, "OK");
  display.display();
}

void initDisplay () {
  Serial.print("Display ");
  pinMode (16, OUTPUT);
  pinMode (2, OUTPUT);
  digitalWrite (16, LOW); // set GPIO16 low to reset OLED
  delay (50);
  digitalWrite (16, HIGH); // while OLED is running, GPIO16 must go high
  display.init ();
  display.flipScreenVertically ();
  display.setFont (ArialMT_Plain_10);
  display.setTextAlignment (TEXT_ALIGN_LEFT);
  display.drawString (0, 0, "DISPLAY");
  display.drawString (100, 0, "OK");
  display.display();
  Serial.println("- OK");
}
// ----------------------------------------------------------------------------- MSP
void initMSP () {
  Serial.print("MSP ");
  display.drawString (0, 8, "MSP ");
  display.display();
  delay(200);
  Serial1.begin(115200, SERIAL_8N1, rxPin , txPin);
  msp.begin(Serial1);
  Serial.println("- OK");
  display.drawString (100, 8, "OK");
  display.display();
}

void getPlaneName () {
  char planeName[20];
  if (msp.request(10, &planeName, sizeof(planeName))) {

    Serial.println(planeName);
  }
}

// ----------------------------------------------------------------------------- main init
void setup() {
  Serial.begin(115200);
  initDisplay();
  initMSP();
  initLora();
  delay(1000);

  getPlaneName();
  uint32_t planeModes;
  msp.getActiveModes(&planeModes);
  Serial.print("Arm State: ");
  Serial.println(bitRead(planeModes,0));
/*
// ----------------------------------------------------------------------------- msp set nav point
  msp_set_wp_t wp;
  wp.waypointNumber = 1;
  wp.action = MSP_NAV_STATUS_WAYPOINT_ACTION_WAYPOINT;
  wp.lat = 501006770;
  wp.lon = 87613380;
  wp.alt = 500;
  wp.p1 = 0;
  wp.p2 = 0;
  wp.p3 = 0;
  wp.flag = 0xa5;
  msp.command(MSP_SET_WP, &wp, sizeof(wp));
// ----------------------------------------------------------------------------- msp get gps
  msp_raw_gps_t gps;
  if (msp.request(MSP_RAW_GPS, &gps, sizeof(gps))) {
    int32_t lat     = gps.lat;
    int32_t lon    = gps.lon;
    int16_t alt      = gps.alt;
    int16_t groundSpeed = gps.groundSpeed;
    Serial.println(gps.numSat);
  }
*/
}

void loop() {
  //LoRa.receive();
}