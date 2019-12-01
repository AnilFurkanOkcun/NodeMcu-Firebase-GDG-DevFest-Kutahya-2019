#include "FirebaseESP8266.h"
#include "ESP8266WiFi.h"
#include "DHTesp.h"

#define PIN_LIGHT D1
#define PIN_FAN D2
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
const String lightStatusPath = "lightStatus";
const String fanStatusPath = "fanStatus";
const String fanTriggerStatusPath = "fanTriggerStatus";
const String fanTriggerMinHeatIndexPath = "fanTriggerMinHeatIndex";

bool lightStatus = false;
bool fanStatus = false;
bool fanTriggerStatus = false;
float lastHeatIndex;
unsigned long lastTemperatureUpdatedTime = 0;

void setup() {
  Serial.begin(115200);
  pinMode(PIN_LIGHT, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  digitalWrite(PIN_LIGHT, !lightStatus);
  digitalWrite(PIN_FAN, !fanStatus);
  connectWifi();
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
}

void loop() {
  // Read LightStatus value
  readLightStatus();
  // Read FanStatus value
  readFanStatus();
  // Read FanTriggerStatus value
  readFanTriggerStatus();
  // Check for FanTriggerMinHeatIndex if temperature updated and Fan is not on and FanTrigger is Enabled
  if (lastTemperatureUpdatedTime != 0 && !fanStatus && fanTriggerStatus ) {
    readFanTriggerMinHeatIndex();
  }
  // Send temperature, humidity, heatIndex
  unsigned long currentTime = millis();
  if (currentTime - lastTemperatureUpdatedTime > temperatureUpdateIntervalMs) {
    lastTemperatureUpdatedTime = currentTime;
    sendTemperature();
  }
}

void readLightStatus() {
  if (Firebase.getBool(firebaseData, lightStatusPath)) {
    bool newLightStatus = firebaseData.boolData();
    // Update lightStatus if necessary
    if (newLightStatus != lightStatus) {
      lightStatus = newLightStatus;
      digitalWrite(PIN_LIGHT, !lightStatus);
    }
  } else {
    onFirebaseError("Read LightStatus");
  }
}

void readFanStatus() {
  if (Firebase.getBool(firebaseData, fanStatusPath)) {
    bool newFanStatus = firebaseData.boolData();
    if (newFanStatus != fanStatus) {
      fanStatus = newFanStatus;
      digitalWrite(PIN_FAN, !fanStatus);
    }
  } else {
    onFirebaseError("Read FanStatus");
  }
}

void readFanTriggerStatus() {
  if (Firebase.getBool(firebaseData, fanTriggerStatusPath)) {
    fanTriggerStatus = firebaseData.boolData();
  } else {
    onFirebaseError("Read FanTriggerStatus");
  }
}


void readFanTriggerMinHeatIndex(){
  if (Firebase.getInt(firebaseData, fanTriggerMinHeatIndexPath)) {
      int fanTriggerMinTemperature = firebaseData.intData();
      // if last calculated heatIndex is bigger than fanTriggerMinTemperature
      if (lastHeatIndex > fanTriggerMinTemperature) {
        // Send FanStatus value true
        sendFanStatus(true);
      }
    } else {
      onFirebaseError("Read FanTriggerMinHeatIndex");
    }
}

void sendFanStatus(bool fanStatus) {
  if (!Firebase.setBool(firebaseData, fanStatusPath, fanStatus )) {
    onFirebaseError("Send FanStatus");
  }
}

void sendTemperature() {
  Serial.println("Sending Temperature");
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  lastHeatIndex = dht.computeHeatIndex(temperature, humidity);
  // Send Humidity value
  if (!Firebase.setFloat(firebaseData, humidityPath, humidity)) {
    onFirebaseError("Send Humidity");
  }
  // Send Temperature value
  if (!Firebase.setFloat(firebaseData, temperaturePath, temperature)) {
    onFirebaseError("TSend emperature");
  }
  // Send HeatIndex value
  if (!Firebase.setFloat(firebaseData, heatIndexPath, lastHeatIndex)) {
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
