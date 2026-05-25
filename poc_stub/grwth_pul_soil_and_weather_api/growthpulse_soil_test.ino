#include <WiFi.h>
#include <Losant.h>

// --- WiFi Credentials ---
const char* WIFI_SSID = "Tech1";
const char* WIFI_PASS = "Challenger2010!";

// --- Losant Credentials ---
const char* LOSANT_DEVICE_ID = "6a0e9065565bf31f4f38000d";
const char* LOSANT_ACCESS_KEY = "2586daa8-94cf-4d89-a1f5-4437ed13581a";
const char* LOSANT_ACCESS_SECRET = "acf9fc5c2b9585bda5ff1646411aef75a133b60d1e0da6e17fa3b1e412200eb8";

// --- Hardware Pins ---
#define MOISTURE_PIN 4  // LM393 Analog Out connected to GPIO 4

// --- Global Objects ---
WiFiClient wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

void connectToWiFi() {
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToLosant() {
  Serial.print("Connecting to Losant...");

  // Connect to Losant using the WiFi client and credentials
  device.connect(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while(!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
}

void setup() {
  Serial.begin(115200);
  delay(100);
  
  // Initialize Analog Pin
  pinMode(MOISTURE_PIN, INPUT);

  // Connect to Network and Platform
  connectToWiFi();
  connectToLosant();
}

void loop() {
  // Reconnect if connection is lost
  bool toReconnect = false;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Disconnected from WiFi");
    toReconnect = true;
  }

  if (!device.connected()) {
    Serial.println("Disconnected from Losant");
    toReconnect = true;
  }

  if (toReconnect) {
    connectToWiFi();
    connectToLosant();
  }

  device.loop();

  // --- Read Sensor ---
  // The ESP32 ADC reads from 0 to 4095. 
  // For the LM393, lower numbers typically mean WET, higher numbers mean DRY.
  int rawMoisture = analogRead(MOISTURE_PIN);
  
  Serial.print("Raw Moisture Reading: ");
  Serial.println(rawMoisture);

  // --- Report to Losant ---
  // We create a JSON object to send the state. 
  // The key "moisture" MUST match the attribute name you created in Losant.
  StaticJsonDocument<200> jsonBuffer;
  JsonObject state = jsonBuffer.to<JsonObject>();
  state["moisture"] = rawMoisture;

  device.sendState(state);
  Serial.println("State reported to Losant.");

  // Wait 15 seconds before sending again to avoid spamming the platform
  delay(15000); 
}
