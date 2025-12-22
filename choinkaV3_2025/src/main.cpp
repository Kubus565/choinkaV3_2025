/*
 * =================================================================
 * PROJEKT: Sterownik Poziomu Wody "Choinka V3"
 * UKŁAD:   ESP32 DevKit V1
 * CZUJNIK: VL53L0X (Laserowy czujnik odległości)
 * =================================================================
 * * SCHEMAT POŁĄCZEŃ:
 * -----------------------------------------------------------------
 * Czujnik VL53L0X      | ESP32 DevKit       | Opis
 * -----------------------------------------------------------------
 * VCC / VIN            | 3V3                | Zasilanie 3.3V
 * GND                  | GND                | Masa
 * SCL                  | GPIO 22            | Linia zegarowa I2C
 * SDA                  | GPIO 21            | Linia danych I2C
 * -----------------------------------------------------------------
 * * FUNKCJE:
 * - Pomiar odległości w czasie rzeczywistym (co 50ms)
 * - Serwer WWW z wizualizacją AJAX (procenty i cm)
 * - Aktualizacja oprogramowania przez WiFi (ArduinoOTA)
 * - Automatyczna kalibracja: 300mm (0%) do 60mm (100%)
 * =================================================================
 */
#include <Wire.h>
#include <VL53L0X.h>
#include <WiFi.h>
#include <ArduinoOTA.h> // Dodana biblioteka OTA

//

void readSensor();
void handleHttp();

VL53L0X sensor;

// Sieć Wi-Fi
const char* ssid = "Orange_Internet_B340";
const char* password = "DtrGc2Nt6ndC7dN739";
WiFiServer server(80);

int currentDistance = 0;
int level = 0;
int lastLevel = -1;

// Kalibracja poziomów
int minLevel = 0;
int maxLevel = 100;
int minDistance = 300; // 0%
int maxDistance = 60;  // 100%

// Zmienne czasowe
unsigned long lastSensorReadTime = 0;
unsigned long lastHttpHandleTime = 0;
const unsigned long sensorInterval = 50;
const unsigned long httpInterval = 50;

void setup() {
  Wire.begin();
  Serial.begin(9600);

  if (!sensor.init()) {
    Serial.println("Błąd czujnika!");
    while (1);
  }
  sensor.startContinuous();
  
  Serial.println("Łączenie z Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nPołączono!");
  Serial.print("Adres IP: ");
  Serial.println(WiFi.localIP());

  // --- KONFIGURACJA OTA ---
  ArduinoOTA.setHostname("Choinka-ESP32"); // Nazwa widoczna w sieci
  
  ArduinoOTA.onStart([]() {
    Serial.println("Start aktualizacji przez WiFi...");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nKoniec aktualizacji!");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Postęp: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Błąd [%u]\n", error);
  });

  ArduinoOTA.begin();
  // ------------------------

  server.begin();
  Serial.println("Serwer HTTP uruchomiony.");
}

void loop() {
  // Obsługa OTA - musi być na samym początku loop
  ArduinoOTA.handle();

  unsigned long currentTime = millis();

  if (currentTime - lastSensorReadTime >= sensorInterval) {
    lastSensorReadTime = currentTime;
    readSensor();
  }

  if (currentTime - lastHttpHandleTime >= httpInterval) {
    lastHttpHandleTime = currentTime;
    handleHttp();
  }

  yield();
}

void readSensor() {
  int rawDistance = sensor.readRangeContinuousMillimeters();
  if (rawDistance > 2000 || sensor.timeoutOccurred()) return;

  currentDistance = rawDistance;
  level = map(currentDistance, minDistance, maxDistance, 0, 100);
  // level = constrain(level, 0, 100);

  if (level != lastLevel) {
    Serial.print("Poziom: "); Serial.print(level); Serial.println("%");
    lastLevel = level;
  }
}

void handleHttp() {
  WiFiClient client = server.available();
  if (!client) return;

  String request = client.readStringUntil('\r');
  client.flush();

  if (request.indexOf("/data") != -1) {
    float currentDistanceCM = currentDistance / 10.0;
    // Wysyłamy level i dystans
    client.print("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\n\r\n" + String(level) + "," + String(currentDistanceCM, 1));
    client.stop();
    return;
  }

  String response = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
  response += "<!DOCTYPE HTML><html><head>";
  response += "<meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1'>";
  response += "<style>";
  response += "body { font-family: sans-serif; text-align: center; background: #1a1a1a; color: white; padding-top: 20px; }";
  response += ".bar-bg { width: 80px; height: 350px; background: #333; margin: 20px auto; border-radius: 40px; position: relative; border: 4px solid #444; overflow: hidden; box-shadow: 0 0 20px rgba(0,0,0,0.5); }";
  response += ".bar-fill { width: 100%; background: linear-gradient(to top, #0077be, #00d4ff); position: absolute; bottom: 0; height: 0%; transition: height 0.4s ease-out; }";
  response += "h1 { font-size: 2.5em; margin-bottom: 10px; }";
  response += "</style></head><body>";
  
  response += "<h1><span id='perc'>0</span>%</h1>";
  response += "<div class='bar-bg'><div id='fill' class='bar-fill'></div></div>";
  response += "<h2><span id='dist'>0</span> cm</h2>";

  response += "<script>";
  response += "setInterval(function() {";
  response += "  fetch('/data').then(r => r.text()).then(data => {";
  response += "    const parts = data.split(',');";
  response += "    document.getElementById('perc').innerHTML = parts[0];";
  response += "    document.getElementById('fill').style.height = parts[0] + '%';";
  response += "    document.getElementById('dist').innerHTML = parts[1];";
  response += "  });";
  response += "}, 200);"; 
  response += "</script></body></html>";

  client.print(response);
  client.stop();
}