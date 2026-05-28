#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// --- Network & API Configuration ---
const char* ssid = "Wokwi-GUEST";
const char* password = "";
const String API_KEY = "65407864a38d3011060f7d2a0dfa2a9a";

// --- Timer Configuration ---
// Using a non-blocking delay so your ESP32 can handle other hardware tasks
unsigned long lastCheckTime = 0;
const unsigned long checkInterval = 600000; // 600,000 ms = 10 minutes

// --- Location Configuration ---
struct Location {
  bool isZip;       // true if using zip code, false if using city/country
  String zip;
  String city;
  String state;
  String country;
};

// Target locations specifically chosen for their high frequency of rain
Location targetLocations[] = {
  {true, "96720", "", "", "US"},                    // Hilo, Hawaii
  {true, "99901", "", "", "US"},                    // Ketchikan, Alaska
  {false, "", "Bergen", "", "NO"},                  // Bergen, Norway
  {false, "", "Quibdo", "", "CO"},                  // Quibdó, Colombia
  {false, "", "Cherrapunjee", "", "IN"}             // Cherrapunji, India
};

const int numLocations = sizeof(targetLocations) / sizeof(targetLocations[0]);

void setup() {
  Serial.begin(115200);
  
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi! IP address: ");
  Serial.println(WiFi.localIP());
  
  // Force an immediate check on startup
  checkWeather();
}

void loop() {
  // Non-blocking timer ensures we only query the API every 10 minutes
  if (millis() - lastCheckTime >= checkInterval) {
    checkWeather();
    lastCheckTime = millis();
  }
  
  // ---> Other hardware logic (sensors, relays) can run freely here <---
}

void checkWeather() {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi Disconnected. Cannot fetch weather.");
    return;
  }

  Serial.println("\n--- Starting Detailed Weather Scan ---");

  for (int i = 0; i < numLocations; i++) {
    HTTPClient http;
    String url = "http://api.openweathermap.org/data/2.5/weather?";

    // Build the URL with units=imperial for Fahrenheit and mph
    if (targetLocations[i].isZip) {
      url += "zip=" + targetLocations[i].zip + "," + targetLocations[i].country + "&appid=" + API_KEY + "&units=imperial";
    } else {
      if (targetLocations[i].state != "") {
        url += "q=" + targetLocations[i].city + "," + targetLocations[i].state + "," + targetLocations[i].country + "&appid=" + API_KEY + "&units=imperial";
      } else {
        url += "q=" + targetLocations[i].city + "," + targetLocations[i].country + "&appid=" + API_KEY + "&units=imperial";
      }
    }

    Serial.print("\nRequesting data for: ");
    if (targetLocations[i].isZip) {
      Serial.println(targetLocations[i].zip);
    } else {
      Serial.println(targetLocations[i].city);
    }

    http.begin(url);
    int httpResponseCode = http.GET();

    if (httpResponseCode == 200) {
      String payload = http.getString();
      
      // Allocate a dynamic JSON document
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);

      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      } else {
        // Extract basic data
        const char* resolvedCity = doc["name"];
        const char* weatherCondition = doc["weather"][0]["main"];
        int weatherId = doc["weather"][0]["id"];
        
        // Extract metrics
        int humidity = doc["main"]["humidity"];
        float temp = doc["main"]["temp"];
        float windSpeed = doc["wind"]["speed"]; 
        
        // Safe extraction for rain (defaults to 0.0 if the object doesn't exist)
        float rain1h = doc["rain"]["1h"] | 0.0; 

        // Print the detailed report
        Serial.printf("--- Weather Report for %s ---\n", resolvedCity);
        Serial.printf("Condition: %s (Code: %d)\n", weatherCondition, weatherId);
        Serial.printf("Temp:      %.1f °F\n", temp);
        Serial.printf("Humidity:  %d%%\n", humidity);
        Serial.printf("Wind:      %.1f mph\n", windSpeed);
        
        if (rain1h > 0) {
            Serial.printf("Rainfall:  %.2f mm/hr\n", rain1h);
            
            // ---> TRIGGER YOUR RAIN-SPECIFIC IOT ACTIONS HERE <---
            
        } else {
            Serial.println("Rainfall:  None in the last hour");
        }
        Serial.println("--------------------------------");
      }
    } else {
      Serial.printf("⚠️ HTTP Error code: %d\n", httpResponseCode);
    }
    
    // Free resources
    http.end(); 
    
    // 1-second delay between individual calls to respect OpenWeather rate limits
    delay(1000); 
  }
  
  Serial.println("\n--- Scan Complete. Waiting 10 minutes. ---");
}