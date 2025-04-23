#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <ArduinoJson.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>
#include <QMC5883LCompass.h>
#include <math.h>

#define RED_LED_PIN 32
#define GREEN_LED_PIN 33
const char* vehicleID = "0000";
const char* SECURITY_ID = "5678";

char ssid[32] = "";
char pass[32] = "";
char inputSecurityID[32] = "";

const char* serverHost = "smart-traffic-its.vercel.app";
const int serverPort = 443;
const char* feedbackPath = "/api/feedback";
const char* gpsPath = "/api/gps";

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);

#define SD_CS 5
File logFile;

unsigned long lastUploadTime = 0;
const unsigned long uploadInterval = 1000;

AsyncWebServer server(80);

#define EEPROM_SIZE 96
#define SSID_ADDR 0
#define PASS_ADDR 32
#define SEC_ID_ADDR 64
#define AP_MODE_FLAG_ADDR (EEPROM_SIZE - 1)

// 🔐 SSL Certificate
const char* rootCACertificate = \
"-----BEGIN CERTIFICATE-----\n" \
"MIIFBjCCAu6gAwIBAgIRAIp9PhPWLzDvI4a9KQdrNPgwDQYJKoZIhvcNAQELBQAw\n" \
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n" \
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjQwMzEzMDAwMDAw\n" \
"WhcNMjcwMzEyMjM1OTU5WjAzMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n" \
"RW5jcnlwdDEMMAoGA1UEAxMDUjExMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIB\n" \
"CgKCAQEAuoe8XBsAOcvKCs3UZxD5ATylTqVhyybKUvsVAbe5KPUoHu0nsyQYOWcJ\n" \
"DAjs4DqwO3cOvfPlOVRBDE6uQdaZdN5R2+97/1i9qLcT9t4x1fJyyXJqC4N0lZxG\n" \
"AGQUmfOx2SLZzaiSqhwmej/+71gFewiVgdtxD4774zEJuwm+UE1fj5F2PVqdnoPy\n" \
"6cRms+EGZkNIGIBloDcYmpuEMpexsr3E+BUAnSeI++JjF5ZsmydnS8TbKF5pwnnw\n" \
"SVzgJFDhxLyhBax7QG0AtMJBP6dYuC/FXJuluwme8f7rsIU5/agK70XEeOtlKsLP\n" \
"Xzze41xNG/cLJyuqC0J3U095ah2H2QIDAQABo4H4MIH1MA4GA1UdDwEB/wQEAwIB\n" \
"hjAdBgNVHSUEFjAUBggrBgEFBQcDAgYIKwYBBQUHAwEwEgYDVR0TAQH/BAgwBgEB\n" \
"/wIBADAdBgNVHQ4EFgQUxc9GpOr0w8B6bJXELbBeki8m47kwHwYDVR0jBBgwFoAU\n" \
"ebRZ5nu25eQBc4AIiMgaWPbpm24wMgYIKwYBBQUHAQEEJjAkMCIGCCsGAQUFBzAC\n" \
"hhZodHRwOi8veDEuaS5sZW5jci5vcmcvMBMGA1UdIAQMMAowCAYGZ4EMAQIBMCcG\n" \
"A1UdHwQgMB4wHKAaoBiGFmh0dHA6Ly94MS5jLmxlbmNyLm9yZy8wDQYJKoZIhvcN\n" \
"AQELBQADggIBAE7iiV0KAxyQOND1H/lxXPjDj7I3iHpvsCUf7b632IYGjukJhM1y\n" \
"v4Hz/MrPU0jtvfZpQtSlET41yBOykh0FX+ou1Nj4ScOt9ZmWnO8m2OG0JAtIIE38\n" \
"01S0qcYhyOE2G/93ZCkXufBL713qzXnQv5C/viOykNpKqUgxdKlEC+Hi9i2DcaR1\n" \
"e9KUwQUZRhy5j/PEdEglKg3l9dtD4tuTm7kZtB8v32oOjzHTYw+7KdzdZiw/sBtn\n" \
"UfhBPORNuay4pJxmY/WrhSMdzFO2q3Gu3MUBcdo27goYKjL9CTF8j/Zz55yctUoV\n" \
"aneCWs/ajUX+HypkBTA+c8LGDLnWO2NKq0YD/pnARkAnYGPfUDoHR9gVSp/qRx+Z\n" \
"WghiDLZsMwhN1zjtSC0uBWiugF3vTNzYIEFfaPG7Ws3jDrAMMYebQ95JQ+HIBD/R\n" \
"PBuHRTBpqKlyDnkSHDHYPiNX3adPoPAcgdF3H2/W0rmoswMWgTlLn1Wu0mrks7/q\n" \
"pdWfS6PJ1jty80r2VKsM/Dj3YIDfbjXKdaFU5C+8bhfJGqU3taKauuz0wHVGT3eo\n" \
"6FlWkWYtbt4pgdamlwVeZEW+LM7qZEJEsMNPrfC03APKmZsJgpWCDWOKZvkZcvjV\n" \
"uYkQ4omYCTX5ohy+knMjdOmdH9c7SpqEWBDC86fiNex+O0XOMEZSa8DA\n" \
"-----END CERTIFICATE-----\n";

WiFiClientSecure client;
MPU6050 mpu(Wire);
QMC5883LCompass compass;

unsigned long lastAccelTime = 0;
float velocityX = 0.0, velocityY = 0.0;
float accelXOffset = 0.0, accelYOffset = 0.0;

void printFeedback(const String& message) {
  Serial.println(message);
}

void setupLeds() {
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  digitalWrite(RED_LED_PIN, LOW);
  digitalWrite(GREEN_LED_PIN, LOW);
}

bool connectToWiFi(const char* newSsid, const char* newPass, const char* newSecurityId, const char* receivedVehicleId) {
  if (!receivedVehicleId || strcmp(receivedVehicleId, vehicleID) != 0) {
    Serial.println("Vehicle ID mismatch! Expected: " + String(vehicleID) + ", Received: " + String(receivedVehicleId ? receivedVehicleId : "null"));
    return false;
  }

  Serial.println("Received credentials:");
  Serial.print("Vehicle ID: ");
  Serial.println(receivedVehicleId ? receivedVehicleId : "null");
  Serial.print("SSID: ");
  Serial.println(newSsid ? newSsid : "null");
  Serial.print("Password: ");
  Serial.println(newPass ? newPass : "null");
  Serial.print("Security ID: ");
  Serial.println(newSecurityId ? newSecurityId : "null");

  if (newSsid && newPass && newSecurityId) {
    if (strcmp(newSecurityId, SECURITY_ID) == 0) {
      EEPROM.write(AP_MODE_FLAG_ADDR, 0);
      for (int i = 0; i < 32; i++) {
        EEPROM.write(SSID_ADDR + i, i < strlen(newSsid) ? newSsid[i] : 0);
        EEPROM.write(PASS_ADDR + i, i < strlen(newPass) ? newPass[i] : 0);
        EEPROM.write(SEC_ID_ADDR + i, i < strlen(newSecurityId) ? newSecurityId[i] : 0);
      }
      EEPROM.commit();
      Serial.println("Credentials stored in EEPROM. Restart will occur soon...");
      return true;
    } else {
      Serial.println("Security ID mismatch!");
      return false;
    }
  } else {
    Serial.println("Invalid configuration data received!");
    return false;
  }
}

void blinkRedLed(unsigned long interval) {
  static unsigned long previousMillis = 0;
  static bool ledState = LOW;

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    ledState = !ledState;
    digitalWrite(RED_LED_PIN, ledState);
  }
}

float getCompassCourse() {
  mpu.update();
  compass.read();

  float ax = mpu.getAccX();
  float ay = mpu.getAccY();
  float az = mpu.getAccZ();

  float normAcc = sqrt(ax * ax + ay * ay + az * az);
  ax /= normAcc;
  ay /= normAcc;
  az /= normAcc;

  float pitch = asin(-ax);
  float roll = asin(ay / cos(pitch));

  float mx = compass.getX();
  float my = compass.getY();
  float mz = compass.getZ();

  float mx2 = mx * cos(pitch) + mz * sin(pitch);
  float my2 = mx * sin(roll) * sin(pitch) + my * cos(roll) - mz * sin(roll) * cos(pitch);

  float heading = atan2(-my2, mx2) * 180.0 / PI;
  if (heading < 0) heading += 360;

  return heading;
}

String getDirectionName(float heading) {
  if (heading >= 337.5 || heading < 22.5) return "North";
  else if (heading < 67.5) return "North-East";
  else if (heading < 112.5) return "East";
  else if (heading < 157.5) return "South-East";
  else if (heading < 202.5) return "South";
  else if (heading < 247.5) return "South-West";
  else if (heading < 292.5) return "West";
  else return "North-West";
}

void setup() {
  Serial.begin(115200);
  setupLeds();
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

  EEPROM.begin(EEPROM_SIZE);

  bool stayInAPMode = EEPROM.read(AP_MODE_FLAG_ADDR) == 1;

  if (stayInAPMode) {
    WiFi.mode(WIFI_AP);
    WiFi.softAP("ESP32_" + String(vehicleID), "12345678");
    IPAddress IP = WiFi.softAPIP();
    Serial.println("Starting in permanent AP mode. AP IP address: " + String(IP));

    server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
      if (request->hasParam("vehicleId", true) && request->hasParam("ssid", true) && 
          request->hasParam("password", true) && request->hasParam("securityId", true)) {
        String newVehicleId = request->getParam("vehicleId", true)->value();
        String newSsid = request->getParam("ssid", true)->value();
        String newPass = request->getParam("password", true)->value();
        String newSecurityId = request->getParam("securityId", true)->value();

        Serial.println("Received credentials:");
        Serial.print("Vehicle ID: ");
        Serial.println(newVehicleId);
        Serial.print("SSID: ");
        Serial.println(newSsid);
        Serial.print("Password: ");
        Serial.println(newPass);
        Serial.print("Security ID: ");
        Serial.println(newSecurityId);

        if (connectToWiFi(newSsid.c_str(), newPass.c_str(), newSecurityId.c_str(), newVehicleId.c_str())) {
          request->send(200, "text/plain", "ACK: Credentials received successfully. Device will restart shortly.");
          
          delay(3000);
          Serial.println("Scheduling restart with new credentials...");
          ESP.restart();
        } else {
          request->send(400, "text/plain", "NACK: Invalid credentials");
        }
      } else {
        request->send(400, "text/plain", "Missing required parameters");
      }
    });

    server.begin();
    Serial.println("HTTP server started on http://192.168.4.1/config");
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    char storedSsid[32] = "";
    char storedPass[32] = "";
    char storedSecId[32] = "";
    for (int i = 0; i < 32; i++) {
      storedSsid[i] = EEPROM.read(SSID_ADDR + i);
      storedPass[i] = EEPROM.read(PASS_ADDR + i);
      storedSecId[i] = EEPROM.read(SEC_ID_ADDR + i);
    }

    if (strlen(storedSsid) > 0 && strlen(storedPass) > 0 && strlen(storedSecId) > 0) {
      Serial.println("Found stored credentials. Attempting STA mode...");
      WiFi.mode(WIFI_STA);
      Serial.println("Attemping to connecting to wifi using credentials:");
      Serial.println(storedSsid);
      Serial.println(storedPass);
      WiFi.begin(storedSsid, storedPass);

      int timeout = 30;
      while (WiFi.status() != WL_CONNECTED && timeout > 0) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        blinkRedLed(500);
        timeout--;
      }

      if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected to WiFi!");
        Serial.println("WiFi IP: " + String(WiFi.localIP()));
      } else {
        Serial.println("Failed to connect to WiFi with stored credentials!");
        for (int i = 0; i < EEPROM_SIZE; i++) {
          EEPROM.write(i, 0);
        }
        EEPROM.write(AP_MODE_FLAG_ADDR, 1);
        EEPROM.commit();
        WiFi.mode(WIFI_AP);
        WiFi.softAP("ESP32_" + String(vehicleID), "12345678");
        IPAddress IP = WiFi.softAPIP();
        Serial.println("Switched to permanent AP mode. AP IP address: " + String(IP));

        server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
          if (request->hasParam("vehicleId", true) && request->hasParam("ssid", true) && 
              request->hasParam("password", true) && request->hasParam("securityId", true)) {
            String newVehicleId = request->getParam("vehicleId", true)->value();
            String newSsid = request->getParam("ssid", true)->value();
            String newPass = request->getParam("password", true)->value();
            String newSecurityId = request->getParam("securityId", true)->value();

            Serial.println("Received credentials:");
            Serial.print("Vehicle ID: ");
            Serial.println(newVehicleId);
            Serial.print("SSID: ");
            Serial.println(newSsid);
            Serial.print("Password: ");
            Serial.println(newPass);
            Serial.print("Security ID: ");
            Serial.println(newSecurityId);

            if (connectToWiFi(newSsid.c_str(), newPass.c_str(), newSecurityId.c_str(), newVehicleId.c_str())) {
              request->send(200, "text/plain", "ACK: Credentials received successfully. Device will restart shortly.");
              
              delay(3000);
              Serial.println("Scheduling restart with new credentials...");
              ESP.restart();
            } else {
              request->send(400, "text/plain", "NACK: Invalid credentials");
            }
          } else {
            request->send(400, "text/plain", "Missing required parameters");
          }
        });

        server.begin();
        Serial.println("HTTP server started on http://192.168.4.1/config");
        digitalWrite(RED_LED_PIN, HIGH);
        digitalWrite(GREEN_LED_PIN, LOW);
      }
    } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP("ESP32_" + String(vehicleID), "12345678");
      IPAddress IP = WiFi.softAPIP();
      Serial.println("Starting in permanent AP mode. AP IP address: " + String(IP));

      server.on("/config", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (request->hasParam("vehicleId", true) && request->hasParam("ssid", true) && 
            request->hasParam("password", true) && request->hasParam("securityId", true)) {
          String newVehicleId = request->getParam("vehicleId", true)->value();
          String newSsid = request->getParam("ssid", true)->value();
          String newPass = request->getParam("password", true)->value();
          String newSecurityId = request->getParam("securityId", true)->value();

          Serial.println("Received credentials:");
          Serial.print("Vehicle ID: ");
          Serial.println(newVehicleId);
          Serial.print("SSID: ");
          Serial.println(newSsid);
          Serial.print("Password: ");
          Serial.println(newPass);
          Serial.print("Security ID: ");
          Serial.println(newSecurityId);

          if (connectToWiFi(newSsid.c_str(), newPass.c_str(), newSecurityId.c_str(), newVehicleId.c_str())) {
            request->send(200, "text/plain", "ACK: Credentials received successfully. Device will restart shortly.");
            
            delay(3000);
            Serial.println("Scheduling restart with new credentials...");
            ESP.restart();
          } else {
            request->send(400, "text/plain", "NACK: Invalid credentials");
          }
        } else {
          request->send(400, "text/plain", "Missing required parameters");
        }
      });

      server.begin();
      Serial.println("HTTP server started on http://192.168.4.1/config");
      digitalWrite(RED_LED_PIN, HIGH);
      digitalWrite(GREEN_LED_PIN, LOW);
    }
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin(SD_CS)) {
    Serial.println("❌ SD Card initialization failed!");
  } else {
    Serial.println("✅ SD Card initialized.");
  }

  Wire.begin(21, 22);
  mpu.begin();
  compass.init();
  compass.setCalibration(-8153, 21422, -8348, 31635, -17020, 10535);

  Serial.println("MPU6050 and Compass initialized");

  float sumX = 0.0, sumY = 0.0;
  int samples = 100;
  for (int i = 0; i < samples; i++) {
    mpu.update();
    sumX += mpu.getAccX();
    sumY += mpu.getAccY();
    delay(50);
  }
  accelXOffset = sumX / samples;
  accelYOffset = sumY / samples;
  Serial.print("Offsets - X: ");
  Serial.print(accelXOffset);
  Serial.print(", Y: ");
  Serial.println(accelYOffset);
  printFeedback("Offsets - X: " + String(accelXOffset) + ", Y: " + String(accelYOffset));

  client.setCACert(rootCACertificate);

  if (client.connect(serverHost, serverPort)) {
    Serial.println("✅ SSL connection test successful");
    client.stop();
  } else {
    Serial.println("❌ SSL connection test failed");
  }

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("Waiting for credentials via HTTP...");
  }
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(RED_LED_PIN, HIGH);
    digitalWrite(GREEN_LED_PIN, LOW);
  } else {
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, HIGH);
  }

  if (WiFi.status() != WL_CONNECTED && EEPROM.read(AP_MODE_FLAG_ADDR) != 1) {
    Serial.println("❌ WiFi not connected - attempting reconnection...");
    WiFi.begin(ssid, pass);
    int reconnectTimeout = 10;
    while (WiFi.status() != WL_CONNECTED && reconnectTimeout > 0) {
      delay(1000);
      Serial.println("Reconnecting to WiFi...");
      blinkRedLed(500);
      digitalWrite(GREEN_LED_PIN, LOW);
      reconnectTimeout--;
    }

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Reconnection failed. Switching to permanent AP mode...");
      for (int i = 0; i < EEPROM_SIZE; i++) {
        EEPROM.write(i, 0);
      }
      EEPROM.write(AP_MODE_FLAG_ADDR, 1);
      EEPROM.commit();
      Serial.println("Restarting in permanent AP mode...");
      delay(1000);
      ESP.restart();
      return;
    }
  }

  if (WiFi.status() == WL_CONNECTED) {
    while (gpsSerial.available() > 0) {
      gps.encode(gpsSerial.read());
    }

    unsigned long currentTime = millis();
    if (currentTime - lastAccelTime >= 100) {
      mpu.update();
      float accelX = (mpu.getAccX() - accelXOffset) * 9.81;
      float accelY = (mpu.getAccY() - accelYOffset) * 9.81;
      float dt = (currentTime - lastAccelTime) / 1000.0;
      velocityX += accelX * dt;
      velocityY += accelY * dt;
      lastAccelTime = currentTime;
    }

    if (gps.satellites.value() == 0) {
      static unsigned int retryCount = 0;
      retryCount++;
      Serial.print("⚠️ No satellites detected! Retry count: ");
      Serial.println(retryCount);
      delay(1000);
      return;
    }

    if (gps.location.isUpdated() && millis() - lastUploadTime >= uploadInterval) {
      lastUploadTime = millis();

      double lat = gps.location.lat();
      double lng = gps.location.lng();
      double spd = gps.speed.kmph();
      double crs = gps.course.deg();

      float accelVelocity = sqrt(velocityX * velocityX + velocityY * velocityY) * 3.6;
      float accelCourse = getCompassCourse();

      int hour = gps.time.hour() + 5;
      int minute = gps.time.minute() + 30;
      if (minute >= 60) {
        hour += 1;
        minute -= 60;
      }
      if (hour >= 24) {
        hour -= 24;
      }

      String timeStr = String(hour) + ":" + (minute < 10 ? "0" : "") + String(minute) + ":" + (gps.time.second() < 10 ? "0" : "") + String(gps.time.second());

      String dataString = "VehicleID: " + String(vehicleID) + ", ";
      dataString += "Latitude: " + String(lat, 6) + ", ";
      dataString += "Longitude: " + String(lng, 6) + ", ";
      dataString += "Speed: " + String(spd) + " km/h, ";
      dataString += "Course: " + String(crs) + "°, ";
      dataString += "AccelVelocity: " + String(accelVelocity) + " km/h, ";
      dataString += "AccelCourse: " + String(accelCourse) + "°, ";
      dataString += "Time: " + timeStr + "\"}";

      Serial.println(dataString);

      logFile = SD.open("/gps_log.txt", FILE_APPEND);
      if (logFile) {
        logFile.println(dataString);
        logFile.close();
        Serial.println("✅ Data saved to SD card");
      } else {
        Serial.println("❌ SD card write failed");
      }

      if (WiFi.status() == WL_CONNECTED) {
        HTTPClient http;
        String serverUrl = "https://" + String(serverHost) + gpsPath;

        client.setTimeout(10000);

        if (!client.connect(serverHost, serverPort)) {
          Serial.println("❌ SSL Connection failed");
          return;
        }

        bool beginSuccess = http.begin(client, serverUrl);
        if (!beginSuccess) {
          Serial.println("❌ HTTP begin failed");
          client.stop();
          return;
        }

        http.addHeader("Content-Type", "application/json");

        String jsonPayload = "{\"vehicleID\":\"" + String(vehicleID) + "\",";
        jsonPayload += "\"latitude\":" + String(lat, 6) + ",";
        jsonPayload += "\"longitude\":" + String(lng, 6) + ",";
        jsonPayload += "\"speed\":" + String(spd) + ",";
        jsonPayload += "\"course\":" + String(crs) + ",";
        jsonPayload += "\"accelVelocity\":" + String(accelVelocity) + ",";
        jsonPayload += "\"accelCourse\":" + String(accelCourse) + ",";
        jsonPayload += "\"time\":\"" + timeStr + "\"}";

        int httpResponseCode = http.POST(jsonPayload);

        if (httpResponseCode > 0) {
          String response = http.getString();
          if (httpResponseCode == 200) {
            Serial.println("✅ Data uploaded to server");
            Serial.print("Server Response: ");
            Serial.println(response);
          } else {
            Serial.print("❌ Upload failed, HTTP code: ");
            Serial.println(httpResponseCode);
            Serial.print("Server Error Message: ");
            Serial.println(response);
          }
        } else {
          Serial.print("❌ Connection failed, HTTP error code: ");
          Serial.println(httpResponseCode);
          if (httpResponseCode == -1) {
            Serial.println("Connection failed or invalid response");
          } else if (httpResponseCode == -2) {
            Serial.println("Send request failed");
          } else if (httpResponseCode == -3) {
            Serial.println("Timeout waiting for response");
          } else if (httpResponseCode == -4) {
            Serial.println("Connection lost");
          } else {
            Serial.println("Unknown error");
          }
        }

        http.end();
        client.stop();
      } else {
        Serial.println("❌ WiFi not connected - attempting reconnection...");
        WiFi.begin(ssid, pass);
        int reconnectTimeout = 10;
        while (WiFi.status() != WL_CONNECTED && reconnectTimeout > 0) {
          delay(1000);
          Serial.println("Reconnecting to WiFi...");
          blinkRedLed(500);
          digitalWrite(GREEN_LED_PIN, LOW);
          reconnectTimeout--;
        }
        if (WiFi.status() != WL_CONNECTED) {
          Serial.println("Reconnection failed. Switching to permanent AP mode...");
          for (int i = 0; i < EEPROM_SIZE; i++) {
            EEPROM.write(i, 0);
          }
          EEPROM.write(AP_MODE_FLAG_ADDR, 1);
          EEPROM.commit();
          Serial.println("Restarting in permanent AP mode...");
          delay(1000);
          ESP.restart();
          return;
        }
      }
    }
  }
}