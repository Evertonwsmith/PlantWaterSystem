#include <Arduino.h>
#include <WiFi.h>

// --- System Configuration Constants ---
const char* const ssid = "WIFI-AE90";
const char* const password = "below9825candid";

// --- Pin Allocations ---
const int SENSOR_PIN   = 34; // Connect the sensor AOUT pin to ESP32 GPIO 34 (ADC1)
const int PWR          = 14; // VCC source for duty-cycling the moisture sensor
const int RELAY_TOGGLE = 13; // Signal line switching your 5V Relay module/LED

NetworkServer server(80);

void setup() {
  Serial.begin(115200);
  
  pinMode(PWR, OUTPUT);
  digitalWrite(PWR, HIGH);         // Power up sensor sub-system
  
  pinMode(RELAY_TOGGLE, OUTPUT);   
  digitalWrite(RELAY_TOGGLE, LOW); // Start with relay safely disengaged
  
  pinMode(SENSOR_PIN, INPUT);
  delay(10);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }

  server.begin();
}

void loop() {
  NetworkClient client = server.accept();  // Listen for incoming browser clients

  if (client) {  
    String currentLine = "";      
    String requestString = ""; // Container to safely hold full target headers
    
    while (client.connected()) {  
      if (client.available()) {   
        char c = client.read();   
        Serial.write(c);          // Optional: mirror raw stream out to Serial Monitor
        requestString += c;

        if (c == '\n') {          // If the byte is a newline character

          // A blank line means the end of the client HTTP request header block:
          if (currentLine.length() == 0) {
            
            // --- ACTION BOUNDARY: Process endpoints cleanly after transmission stops ---
            if (requestString.indexOf("GET /H") >= 0) {
              // Read raw voltage telemetry from your capacitive sensor
              int rawValue = analogRead(SENSOR_PIN);
              
              Serial.println("\n******************************");
              Serial.print("[SENSOR] Raw ADC Value: ");
              Serial.println(rawValue);
              Serial.println("******************************");
              
              digitalWrite(RELAY_TOGGLE, HIGH);  // Turn relay module ON
            }
            
            if (requestString.indexOf("GET /L") >= 0) {
              digitalWrite(RELAY_TOGGLE, LOW);   // Turn relay module OFF
              Serial.println("\n[ACTION] Relay forced OFF via /L command.");
            }

            // Transmit HTTP response headers back to browser
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // Render interactive UI links back onto the user device viewport
            client.print("Click <a href=\"/H\">here</a> to turn on and read sensor.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn off.<br>");

            client.println();
            break; // Break out of the processing loops smoothly
            
          } else {  
            currentLine = ""; // Clear line tracker buffer on normal linebreaks
          }
        } else if (c != '\r') {  
          currentLine += c;      
        }
      }
    }
    // Safely drop the client channel to clear memory for the next loop cycle
    delay(10); 
    client.stop();
  }
}