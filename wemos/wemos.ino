#include "EEPROM.h"
#include "ESP8266WebServer.h"
#include "DNSServer.h"
#include "FS.h"

#define SSID "tiger-board"
#define PASSWORD ""

#define CON_PIN D1
#define CON_BAUDRATE 19200

IPAddress IP(192, 168, 4, 1);
IPAddress NETMASK(255, 255, 255, 0);

DNSServer dns;
ESP8266WebServer http(80);

uint8_t mode = 0;
uint8_t speed = 0;
uint32_t color[] = { 0, 0, 0, 0 };

void setup() {
  // prepare
  SPIFFS.begin();

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(IP, IP, NETMASK);
  WiFi.softAP(SSID, PASSWORD);

  dns.setErrorReplyCode(DNSReplyCode::NoError);
  dns.start(53, "*", IP);

  http.serveStatic("/", SPIFFS, "/index.html");
  http.serveStatic("/bootstrap.css", SPIFFS, "/bootstrap.css");
  http.serveStatic("/slick.css", SPIFFS, "/slick.css");
  http.serveStatic("/board.css", SPIFFS, "/board.css");
  http.serveStatic("/jquery.js", SPIFFS, "/jquery.js");
  http.serveStatic("/slick.js", SPIFFS, "/slick.js");
  http.serveStatic("/board.js", SPIFFS, "/board.js");
  http.on("/get", handleGet);
  http.on("/set", handleSet);
  http.onNotFound(handle404);
  http.begin();

  pinMode(D1, OUTPUT);

  // read last configuration
  EEPROM.begin(512);
  
  mode = EEPROM.read(0);
  speed = EEPROM.read(1);

  uint8_t p = 2;
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t k = 0; k < 24; k += 8) {
      color[i] |= ((uint32_t) EEPROM.read(p++) << k);
    }
  }

  EEPROM.end();

  // send to arduino
  delay(4000);
  sendUpdate();
}

void loop() {
  dns.processNextRequest();
  http.handleClient();
}

void sendUpdate() {
  Serial.begin(CON_BAUDRATE);
  digitalWrite(CON_PIN, HIGH);

  delay(100);
  digitalWrite(CON_PIN, LOW);

  Serial.write(mode);
  delay(25);
  
  Serial.write(speed);
  delay(25);

  uint8_t p = 2;
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t k = 0; k < 24; k += 8) {
      Serial.write(color[i] >> k);
      delay(25);
    }
  }

  Serial.end();
}

void handleGet() {
  http.send(200, "application/json", "{\"mode\":" + String(mode) + ",\"speed\":" + String(speed) + ",\"color\":[" + String(color[0]) + "," + String(color[1]) + "," + String(color[2]) + "," + String(color[3]) + "]}");
}

void handleSet() {
  // handle request
  for (uint8_t i = 0; i < http.args(); i++)
    if (http.argName(i).equals("mode"))
      mode = http.arg(i).toInt();
    else if (http.argName(i).equals("speed"))
      speed = http.arg(i).toInt();
    else if (http.argName(i).equals("color0"))
      color[0] = http.arg(i).toInt();
    else if (http.argName(i).equals("color1"))
      color[1] = http.arg(i).toInt();
    else if (http.argName(i).equals("color2"))
      color[2] = http.arg(i).toInt();
    else if (http.argName(i).equals("color3"))
      color[3] = http.arg(i).toInt();

  // write to eeprom
  EEPROM.begin(512);
    
  EEPROM.write(0, mode);
  EEPROM.write(1, speed);

  uint8_t p = 2;
  for (uint8_t i = 0; i < 4; i++) {
    for (uint8_t k = 0; k < 24; k += 8) {
      EEPROM.write(p++, color[i] >> k);
    }
  }

  EEPROM.commit();
  EEPROM.end();
    
  // send update and return success
  sendUpdate();
  http.send(200, "application/json", "{}");
}

void handle404() {
  http.sendHeader("Location", String("/"), true);
  http.send(302, "text/plain", "");
}

