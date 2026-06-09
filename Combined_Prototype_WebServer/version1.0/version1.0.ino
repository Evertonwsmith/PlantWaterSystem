#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>  // Standard, stable library
#include "index.h"
#include <DHT.h>
#define DHTPIN 25      // The digital pin connected to the DHT11
#define DHTTYPE DHT11  // Define the sensor type

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char* ssid = "WIFI-AE90";
const char* password = "below9825candid";
const int SENSOR_PIN = 32;
const int RELAY_TRIG = 27;
const int SENSOR_PWR = 33;
const int TEMP_PWR = 26;
const char* PARAM_MESSAGE = "message";

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

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
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("\nIP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });
  server.on("/temperature", HTTP_GET, []() {
    // Replace 24.5 with your actual sensor reading variable (e.g., dht.readTemperature())
    float currentTemp = dht.readTemperature();
    server.send(200, "text/plain", String(currentTemp) + " °C");
  });

  server.on("/humidity", HTTP_GET, []() {
    // Replace 60.0 with your actual sensor reading variable (e.g., dht.readHumidity())
    float currentHumidity = dht.readHumidity();
    server.send(200, "text/plain", String(currentHumidity) + " %");
  });

  server.on("/moisture", HTTP_GET, []() {
    // Replace 45 with your actual soil moisture reading variable
    int currentMoisture = analogRead(SENSOR_PIN);
    server.send(200, "text/plain", String(currentMoisture) + " %");
  });

  // ---- POST Endpoints for Control Buttons ----
  // Your HTML also expects these POST endpoints for the buttons and scheduler to work

  server.on("/water-now", HTTP_POST, []() {
    Serial.println("Action: Watering started via web control!");
    // Add your relay/pump code here
    digitalWrite(RELAY_TRIG, LOW);
    delay(2000);
    digitalWrite(RELAY_TRIG, HIGH);
    server.send(200, "text/plain", "Watering cycle triggered");
  });
  
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();  // This takes care of processing incoming requests safely
  // updateData();
}