# 🎄 Choinka V3 - Inteligentny Monitor Poziomu Wody

Projekt opartego na **ESP32** systemu monitorowania poziomu wody (np. w stojaku na choinkę) z bezprzewodową wizualizacją w czasie rzeczywistym.
<img width="946" height="2047" alt="image" src="https://github.com/user-attachments/assets/ecd66b34-6530-40e3-8e7b-5168c055bde4" />


## 🚀 Funkcje
* **Pomiar laserowy:** Wykorzystanie czujnika VL53L0X (ToF) dla precyzji co do 1 mm.
* **Interfejs WWW:** dynamiczny pasek postępu (AJAX) odświeżany co 200ms (bez przeładowywania strony).
* **Jednostki:** Wyświetlanie poziomu w procentach (0-100%) oraz odległości w centymetrach (cm).
* **OTA (Over-The-Air):** Możliwość aktualizacji kodu przez WiFi – nie trzeba podłączać kabla USB do choinki.
* **Responsywny Design:** Strona WWW dopasowana do ekranów telefonów i komputerów (tryb Dark Mode).

## 🛠️ Schemat połączeń

| Czujnik VL53L0X | ESP32 DevKit | Opis |
| :--- | :--- | :--- |
| **VCC** | **3V3** | Zasilanie 3.3V |
| **GND** | **GND** | Masa |
| **SCL** | **GPIO 22** | Linia zegarowa I2C |
| **SDA** | **GPIO 21** | Linia danych I2C |



## ⚙️ Kalibracja
Wartości można dostosować w kodzie źródłowym:
* `minDistance = 300` (30 cm od czujnika = 0% wody)
* `maxDistance = 60` (6 cm od czujnika = 100% wody)

## 💻 Technologia
* **Framework:** Arduino (PlatformIO)
* **Język:** C++ / HTML / CSS / JavaScript (AJAX)
* **Biblioteki:** * `pololu/VL53L0X`
  * `WiFi`
  * `ArduinoOTA`

## 📦 Instalacja
1. Sklonuj repozytorium.
2. Skonfiguruj dane sieci WiFi w pliku `main.cpp`.
3. Wgraj kod przez USB (pierwszy raz).
4. Kolejne aktualizacje wykonuj przez OTA, ustawiając adres IP w `platformio.ini`:
   ```ini
   upload_protocol = espota
   upload_port = YOUR_ESP32_IP
