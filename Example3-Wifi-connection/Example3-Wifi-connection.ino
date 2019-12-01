#include "ESP8266WiFi.h"

#define WIFI_SSID "wifi_name"
#define WIFI_PASSWORD "wifi_password"

void setup() {
  Serial.begin(115200);
  connectWifi();
}

void loop() {
}

void connectWifi() {
  Serial.printf("Connecting to %s \n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...");
    delay(250);
  }
  Serial.println("Connected to WiFi");
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
}
