#include "FirebaseESP8266.h"
#include "ESP8266WiFi.h"

#define PIN_LED D1

#define WIFI_SSID "wifi_name"
#define WIFI_PASSWORD "wifi_password"

#define FIREBASE_HOST "firebase_project_id.firebaseio.com"
#define FIREBASE_AUTH "firebase_database_secret"

FirebaseData firebaseData;

const String ledStatusPath = "ledStatus";

bool ledStatus = false;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, ledStatus);
  connectWifi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Read LedStatus value
  readLedStatus();
}

void readLedStatus() {
  if (Firebase.getBool(firebaseData, ledStatusPath)) {
    bool newLedStatus = firebaseData.boolData();
    // Update LedStatus if necessary
    if (newLedStatus != ledStatus) {
      ledStatus = newLedStatus;
      digitalWrite(PIN_LED, ledStatus);
    }
  } else {
    onFirebaseError("Read LedStatus");
  }
}

void onFirebaseError(String action) {
  Serial.print("Firebase Error: "+action);
  Serial.println(firebaseData.errorReason());
}

void connectWifi() {
  Serial.printf("Connecting to %s \n", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("...");
    delay(500);
  }
  Serial.println("Connected to WiFi");
  Serial.printf("Local IP: %s\n", WiFi.localIP().toString().c_str());
}
