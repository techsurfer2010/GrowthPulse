#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <Losant.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>


// ======================================================
// Pin Definitions
// ======================================================

#define ONE_WIRE_BUS 4
#define DHTPIN 5
#define DHTTYPE DHT22
#define SOIL_PIN 1

// ======================================================
// Soil Moisture Calibration
// ======================================================

// Adjust these after calibration
int dryValue = 3600;
int wetValue = 1300;

// ======================================================
// Sensor Objects
// ======================================================

// DS18B20
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

// DHT22
DHT dht(DHTPIN, DHTTYPE);

// ======================================================
// Global Sensor Variables
// ======================================================

float soilTemperatureF = 0.0;

float airTemperatureF = 0.0;
float airHumidity = 0.0;

int soilRaw = 0;
int soilMoisturePercent = 0;

// ======================================================
// WiFi Credentials
// ======================================================

const char* WIFI_SSID = "ADD WIFI DATA";
const char* WIFI_PASS = "ADD WIFI DATA";

// ======================================================
// Losant Credentials
// ======================================================

const char* LOSANT_DEVICE_ID =
  "LOSANT DEVICE";

const char* LOSANT_ACCESS_KEY =
  "LOSANT ACCESS KEY";

const char* LOSANT_ACCESS_SECRET =
  "LOSANT ACCESS SECRET";

// For an unsecure connection to Losant.
WiFiClient wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);

// ======================================================
// Setup
// ======================================================

void setup() {

  Serial.begin(115200);

  // ESP32 ADC resolution
  analogReadResolution(12);

  delay(1000);

  Serial.println();
  Serial.println("=================================");
  Serial.println("Environmental Sensor Node");
  Serial.println("=================================");

  initializeSensors();
  connectToWiFiAndLosant();
}

// ======================================================
// Wifi and Losant Connection
// ======================================================
void connectToWiFiAndLosant() {

  Serial.println();
  Serial.print("Connecting to WiFi");

  WiFi.persistent(false);

  WiFi.mode(WIFI_OFF);

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");

  Serial.print("IP Address: ");

  Serial.println(WiFi.localIP());

  // --------------------------------------------------

  Serial.print("Connecting to Losant...");

  device.connect(
    wifiClient,
    LOSANT_ACCESS_KEY,
    LOSANT_ACCESS_SECRET
  );

  while (!device.connected()) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("Connected to Losant");
}


// ======================================================
// Losant Telemetry
// ======================================================
void sendTelemetryToLosant() {

  StaticJsonDocument<500> jsonBuffer;

  JsonObject root =
    jsonBuffer.to<JsonObject>();

  root["soilTemperatureF"] =
    soilTemperatureF;

  root["airTemperatureF"] =
    airTemperatureF;

  root["airHumidity"] =
    airHumidity;

  root["soilRaw"] =
    soilRaw;

  root["soilMoisturePercent"] =
    soilMoisturePercent;

  device.sendState(root);

  Serial.println("Telemetry sent to Losant");
}

// ======================================================
// Main Loop
// ======================================================

void loop() {

  readSoilTemperature();

  readEnvironmentalSensor();

  readSoilMoisture();

  printTelemetry();

  if (!device.connected()) {

    Serial.println("Reconnecting to Losant...");

    connectToWiFiAndLosant();
  }

  device.loop();

  sendTelemetryToLosant();

  delay(2000);
}

// ======================================================
// Sensor Initialization
// ======================================================

void initializeSensors() {

  ds18b20.begin();

  dht.begin();

  Serial.println("Sensors initialized");
}

// ======================================================
// DS18B20
// ======================================================

void readSoilTemperature() {

  ds18b20.requestTemperatures();

  soilTemperatureF = ds18b20.getTempFByIndex(0);
}

// ======================================================
// DHT22
// ======================================================

void readEnvironmentalSensor() {

  airTemperatureF = dht.readTemperature(true);

  airHumidity = dht.readHumidity();
}

// ======================================================
// Soil Moisture
// ======================================================

void readSoilMoisture() {

  soilRaw = 0;

  // Average multiple readings for stability
  for (int i = 0; i < 10; i++) {

    soilRaw += analogRead(SOIL_PIN);

    delay(10);
  }

  soilRaw /= 10;

  // Convert to percentage
  soilMoisturePercent = map(
    soilRaw,
    dryValue,
    wetValue,
    0,
    100
  );

  soilMoisturePercent = constrain(
    soilMoisturePercent,
    0,
    100
  );
}

// ======================================================
// Telemetry Output
// ======================================================

void printTelemetry() {

  Serial.println();
  Serial.println("---------------------------");

  // ----- Soil Temperature -----

  Serial.print("Soil Temperature: ");

  Serial.print(soilTemperatureF);

  Serial.println(" F");

  // ----- Environmental Sensor -----

  if (isnan(airTemperatureF) || isnan(airHumidity)) {

    Serial.println("DHT22 read failed!");

  } else {

    Serial.print("Environmental Temperature: ");

    Serial.print(airTemperatureF);

    Serial.println(" F");

    Serial.print("Environmental Humidity: ");

    Serial.print(airHumidity);

    Serial.println(" %");
  }

  // ----- Soil Moisture -----

  Serial.print("Soil Raw: ");

  Serial.println(soilRaw);

  Serial.print("Soil Moisture: ");

  Serial.print(soilMoisturePercent);

  Serial.println(" %");
}
