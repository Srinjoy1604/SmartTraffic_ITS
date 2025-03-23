#include <HardwareSerial.h>
#include <TinyGPS++.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SPI.h>
#include <SD.h>

const char* ssid = "OnePlus 11R 5G";
const char* password = "123456789@";

const char* serverHost = "smart-traffic-its.vercel.app";
const int serverPort = 443;
const char* serverPath = "/api/gps";

#define RXD2 16
#define TXD2 17
#define GPS_BAUD 9600
TinyGPSPlus gps;
HardwareSerial gpsSerial(2);  // UART2 for GPS

#define SD_CS 5
File logFile;

unsigned long lastUploadTime = 0;
const unsigned long uploadInterval = 5000;

const String vehicleID = "0000";

// 🔐 SSL Certificate (get from openssl s_client)
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

void setup() {
    Serial.begin(115200);
    gpsSerial.begin(GPS_BAUD, SERIAL_8N1, RXD2, TXD2);

    WiFi.begin(ssid, password);
    Serial.print("Connecting to WiFi");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\n✅ WiFi connected");

    Serial.print("Initializing SD card...");
    if (!SD.begin(SD_CS)) {
        Serial.println("❌ SD Card initialization failed!");
    } else {
        Serial.println("✅ SD Card initialized.");
    }

    // ✅ Use proper CA certificate
    client.setCACert(rootCACertificate);

    if (client.connect(serverHost, serverPort)) {
        Serial.println("✅ SSL connection test successful");
        client.stop();
    } else {
        Serial.println("❌ SSL connection test failed");
    }
}

void loop() {
    while (gpsSerial.available() > 0) {
        gps.encode(gpsSerial.read());
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

        String dataString = "VehicleID: " + vehicleID + ", ";
        dataString += "Latitude: " + String(lat, 6) + ", ";
        dataString += "Longitude: " + String(lng, 6) + ", ";
        dataString += "Speed: " + String(spd) + " km/h, ";
        dataString += "Course: " + String(crs) + "°, ";
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
            Serial.println("🌐 Attempting HTTPS connection to smart-traffic-its.vercel.app...");

            String serverUrl = "https://";
            serverUrl += serverHost;
            serverUrl += serverPath;

            client.setTimeout(10000);  // 10 seconds timeout
            
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

            String jsonPayload = "{\"vehicleID\":\"" + vehicleID + "\",";
            jsonPayload += "\"latitude\":" + String(lat, 6) + ",";
            jsonPayload += "\"longitude\":" + String(lng, 6) + ",";
            jsonPayload += "\"speed\":" + String(spd) + ",";
            jsonPayload += "\"course\":" + String(crs) + ",";
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
                // Generic error handling without specific constants
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
            Serial.println("❌ WiFi not connected");
        }
    }
}
