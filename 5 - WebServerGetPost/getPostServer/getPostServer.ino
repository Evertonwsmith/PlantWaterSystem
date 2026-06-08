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

  server.on("/get", HTTP_GET, []() {
    String message = server.hasArg(PARAM_MESSAGE) ? server.arg(PARAM_MESSAGE) : "No message sent";
    server.send(200, "text/plain", "Hello, GET: " + message);
  });

  server.on("/post", HTTP_POST, []() {
    String message = server.hasArg(PARAM_MESSAGE) ? server.arg(PARAM_MESSAGE) : "No message sent";
    server.send(200, "text/plain", "Hello, POST: " + message);
  });

  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();  // This takes care of processing incoming requests safely
}