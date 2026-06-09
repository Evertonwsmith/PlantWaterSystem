#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>  // Standard, stable library
#include "index.h"
WebServer server(80);

const char* ssid = "WIFI-AE90";
const char* password = "below9825candid";

const char* PARAM_MESSAGE = "message";

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

void setup() {
  Serial.begin(115200);
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
    float currentTemp = 24.5;
    server.send(200, "text/plain", String(currentTemp) + " °C");
  });

  server.on("/humidity", HTTP_GET, []() {
    // Replace 60.0 with your actual sensor reading variable (e.g., dht.readHumidity())
    float currentHumidity = 60.0;
    server.send(200, "text/plain", String(currentHumidity) + " %");
  });

  server.on("/moisture", HTTP_GET, []() {
    // Replace 45 with your actual soil moisture reading variable
    int currentMoisture = 45;
    server.send(200, "text/plain", String(currentMoisture) + " %");
  });

  // ---- POST Endpoints for Control Buttons ----
  // Your HTML also expects these POST endpoints for the buttons and scheduler to work

  server.on("/water-now", HTTP_POST, []() {
    Serial.println("Action: Watering started via web control!");
    // Add your relay/pump code here
    server.send(200, "text/plain", "Watering cycle triggered");
  });

  server.on("/pump-on", HTTP_POST, []() {
    Serial.println("Action: Pump turned ON manually!");
    // Add your relay/pump code here
    server.send(200, "text/plain", "Pump is now ON");
  });

  server.on("/pump-off", HTTP_POST, []() {
    Serial.println("Action: Pump turned OFF manually!");
    // Add your relay/pump code here
    server.send(200, "text/plain", "Pump is now OFF");
  });
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();  // This takes care of processing incoming requests safely
  updateData();
}