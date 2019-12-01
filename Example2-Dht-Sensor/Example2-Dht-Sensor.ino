#include "DHTesp.h"

#define PIN_DHT D0

DHTesp dht;

void setup() {
  Serial.begin(115200);
  dht.setup(PIN_DHT, DHTesp::DHT11);
}

void loop() {
  float temperature = dht.getTemperature();
  float humidity = dht.getHumidity();
  float heatIndex = dht.computeHeatIndex(temperature, humidity);
  Serial.println("Sıcaklık: "+(String) temperature+"°C   Nem Oranı: %"+(String) humidity+"   Hissedilen Sıcaklık: "+(String) heatIndex+"°C");
  delay(5000);
}
