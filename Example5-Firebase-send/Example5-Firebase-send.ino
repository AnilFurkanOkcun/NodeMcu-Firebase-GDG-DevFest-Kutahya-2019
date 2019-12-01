#include "FirebaseESP8266.h"
#include "ESP8266WiFi.h"
#include "DHTesp.h"

#define PIN_LED D1
#define PIN_DHT D0

#define WIFI_SSID "wifi_name"
#define WIFI_PASSWORD "wifi_password"

#define FIREBASE_HOST "firebase_project_id.firebaseio.com"
#define FIREBASE_AUTH "firebase_database_secret"

FirebaseData firebaseData;

DHTesp dht;

const int temperatureUpdateIntervalMs = 30 * 1000; // 30 Seconds
const String temperaturePath = "temperature";
const String humidityPath = "humidity";
const String heatIndexPath = "heatIndex";
const String ledStatusPath = "ledStatus";

bool ledStatus = false;
unsigned long lastTemperatureUpdatedTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LED, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  digitalWrite(PIN_LED, ledStatus);
  connectWifi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Read LedStatus value
  readLedStatus();
  // Send temperature, humidity, heatIndex
  unsigned long currentTime = millis();
  if (currentTime - lastTemperatureUpdatedTime > temperatureUpdateIntervalMs) {
    lastTemperatureUpdatedTime = currentTime;
    sendTemperature();
  }
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

void sendTemperature() {
  Serial.println("Sending Temperature");
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  float heatIndex = dht.computeHeatIndex(temperature, humidity);
  // Send Humidity value
  if (!Firebase.setFloat(firebaseData, humidityPath, humidity)) {
    onFirebaseError("Send Humidity");
  }
  // Send Temperature value
  if (!Firebase.setFloat(firebaseData, temperaturePath, temperature)) {
    onFirebaseError("TSend emperature");
  }
  // Send HeatIndex value
  if (!Firebase.setFloat(firebaseData, heatIndexPath, heatIndex)) {
    onFirebaseError("Send HeatIndex");
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
