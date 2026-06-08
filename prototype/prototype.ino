#include <Arduino.h>
#include <WiFi.h>
#include <DHT.h>

#define DHTPIN 25      // The digital pin connected to the DHT11
#define DHTTYPE DHT11  // Define the sensor type

DHT dht(DHTPIN, DHTTYPE);
// --- System Configuration Constants ---
const char* const ssid = "WIFI-AE90";
const char* const password = "below9825candid";
const int SENSOR_PIN = 32;
const int RELAY_TRIG = 27;
const int SENSOR_PWR = 33;
const int TEMP_PWR = 26;

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PWR, OUTPUT);
  digitalWrite(SENSOR_PWR, HIGH);
  pinMode(TEMP_PWR, OUTPUT);
  digitalWrite(TEMP_PWR, HIGH);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(RELAY_TRIG, OUTPUT);
  digitalWrite(RELAY_TRIG, HIGH);
  dht.begin();
}

void loop() {


  // Serial.print("READ:  ");
  // Serial.print(analogRead(SENSOR_PIN));
  // Serial.println("\n");
  // int moisture = analogRead(SENSOR_PIN);
  // if (moisture > 2000) {
  //   digitalWrite(RELAY_TRIG, LOW);
  //   delay(1000);
  // } else {
  //   float humidity = dht.readHumidity();
  //   float temperature = dht.readTemperature();  // Celcius by default

  //   // Check if any reads failed
  //   if (isnan(humidity) || isnan(temperature)) {
  //     Serial.println("Failed to read from DHT sensor!");
  //   } else {
  //     Serial.print("Humidity: ");
  //     Serial.print(humidity);
  //     Serial.print("%  |  Temperature: ");
  //     Serial.print(temperature);
  //     Serial.println("°C");
  //   }
  //   digitalWrite(RELAY_TRIG, HIGH);
  //   delay(1000);
  // }
}
