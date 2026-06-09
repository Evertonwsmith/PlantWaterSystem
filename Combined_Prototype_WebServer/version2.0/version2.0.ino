#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

// --- HTML Content Wrapper (Usually housed in index.h) ---
// If you are putting this in a separate index.h file, remove this string declaration.
const char index_html[] PROGMEM = R"rawliteral(
)rawliteral";

#define DHTPIN 25      // The digital pin connected to the DHT11
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);

const char* ssid = "WIFI-AE90";
const char* password = "below9825candid";
const int SENSOR_PIN = 32;
const int RELAY_TRIG = 27;
const int SENSOR_PWR = 33;
const int TEMP_PWR = 26;

// --- Time and Schedule Settings ---
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = -18000;      // Adjust for your Timezone in seconds (e.g., -18000 = EST)
const int   daylightOffset_sec = 3600;   // Daylight savings offset (3600 seconds or 0 if not used)
String lastTriggeredTime = "";           // Latches to prevent double triggers in a single minute

void handleNotFound() {
  server.send(404, "text/plain", "Not found");
}

// Triggers active watering cycle
void triggerWatering() {
  Serial.println("Action: Watering started via control!");
  digitalWrite(RELAY_TRIG, LOW);
  delay(2000);
  digitalWrite(RELAY_TRIG, HIGH);
}

// Reads schedule string saved inside LittleFS storage
String loadScheduleFromFile() {
  if (!LittleFS.exists("/schedule.json")) {
    return "[]"; 
  }
  File file = LittleFS.open("/schedule.json", "r");
  if (!file) return "[]";
  String json = file.readString();
  file.close();
  return json;
}

// Writes schedule string down to LittleFS storage area
void saveScheduleToFile(String jsonString) {
  File file = LittleFS.open("/schedule.json", "w");
  if (!file) {
    Serial.println("Failed to write schedule to file system.");
    return;
  }
  file.print(jsonString);
  file.close();
  Serial.println("Schedule written successfully.");
}

// Checks current real-time against stored JSON matrix
void checkSchedule() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    // Time not initialized yet
    return;
  }

  // Generate current timestamp matching "HH:MM"
  char timeBuf[6];
  strftime(timeBuf, sizeof(timeBuf), "%H:%M", &timeinfo);
  String currentTime = String(timeBuf);

  // If already triggered on this exact minute mark, stop to prevent loops
  if (currentTime == lastTriggeredTime) return;

  String json = loadScheduleFromFile();
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  if (error) return;

  JsonArray array = doc.as<JsonArray>();
  for (JsonVariant v : array) {
    bool enabled = v["enabled"] | false;
    const char* timeStr = v["time"] | "";

    if (enabled && String(timeStr) == currentTime) {
      Serial.printf("⏰ Clock Match! Automatic watering activated for %s\n", timeStr);
      lastTriggeredTime = currentTime; // Latch
      triggerWatering();
      break; 
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  // Initializing Mount points
  if (!LittleFS.begin(true)) {
    Serial.println("An Error has occurred while mounting LittleFS. Formatting completed.");
  }

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

  // Configure Core Clock Architecture with local NTP configuration
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println("NTP Time synchronizing initiated...");

  // --- REST Routes ---

  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", index_html);
  });

  server.on("/temperature", HTTP_GET, []() {
    float currentTemp = dht.readTemperature();
    server.send(200, "text/plain", String(currentTemp, 1) + " °C");
  });

  server.on("/humidity", HTTP_GET, []() {
    float currentHumidity = dht.readHumidity();
    server.send(200, "text/plain", String(currentHumidity, 1) + " %");
  });

  server.on("/moisture", HTTP_GET, []() {
    int currentMoisture = analogRead(SENSOR_PIN);
    // Optional: Translate your analog value to percentage if mapped properly
    server.send(200, "text/plain", String(currentMoisture));
  });

  server.on("/water-now", HTTP_POST, []() {
    triggerWatering();
    server.send(200, "text/plain", "Watering cycle triggered");
  });

  // GET API Endpoint: Sends flash payload configuration array out to browser
  server.on("/api/schedule", HTTP_GET, []() {
    String jsonPayload = loadScheduleFromFile();
    server.send(200, "application/json", jsonPayload);
  });

  // POST API Endpoint: Captures browser user schedule lists, saves directly to disk
  server.on("/api/schedule", HTTP_POST, []() {
    if (!server.hasArg("plain")) {
      server.send(400, "text/plain", "Payload Missing");
      return;
    }
    String postedData = server.arg("plain");
    saveScheduleToFile(postedData);
    server.send(200, "text/plain", "Schedule updated on hardware file system.");
  });
  
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop() {
  server.handleClient();

  // Non-blocking timer check loop execution occurring every 5 seconds
  static unsigned long lastTimeCheck = 0;
  if (millis() - lastTimeCheck > 5000) {
    lastTimeCheck = millis();
    checkSchedule();
  }
}