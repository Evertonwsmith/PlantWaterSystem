#include <Arduino.h>
#include <WiFi.h>

// --- System Configuration Constants ---
const char* const ssid = "WIFI-AE90";
const char* const password = "below9825candid";
const int SENSOR_PIN = 34;  // Connect the sensor AOUT pin to ESP32 GPIO 34
const int SENSOR_PWR = 14;
const int LED = 25;
NetworkServer server(80);

void setup() {
  Serial.begin(115200);
  pinMode(SENSOR_PWR, OUTPUT);
  pinMode(SENSOR_PIN, INPUT);
  pinMode(LED, OUTPUT);
  delay(10);
  digitalWrite(SENSOR_PWR, HIGH);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  //Serial.println("");
  //Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();
}

void loop() {
  NetworkClient client = server.accept();  // listen for incoming clients

  if (client) {  // if you get a client,

    //Serial.println("New Client.");  // print a message out the serial port
    String currentLine = "";      // make a String to hold incoming data from the client
    while (client.connected()) {  // loop while the client's connected
      if (client.available()) {   // if there's bytes to read from the client,
        client.println("HTTP/1.1 200 OK");
        client.println("Content-type:text/html");
        client.println();

        //     // the content of the HTTP response follows the header:
        client.print("Click <a href=\"/H\"><h1>ON</h1>/a><br>");
        client.print("Click <a href=\"/L\"><h1>OFF</h1></a><br>");

        // The HTTP response ends with another blank line:
        client.println();

        int rawValue = analogRead(SENSOR_PIN);
        Serial.print("Raw ADC Value: ");
        Serial.println(rawValue);
        if (rawValue > 900) {
          digitalWrite(LED, LOW);
        } else {
          digitalWrite(LED, HIGH)
        }
        // }
        // if (currentLine.endsWith("GET /L")) {
        //   digitalWrite(13, LOW);  // GET /L turns the LED off
        // }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("Client Disconnected.");
  }
}
