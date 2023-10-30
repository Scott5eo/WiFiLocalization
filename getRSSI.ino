#include "WiFi.h"

void setup()
{
    Serial.begin(115200);
    // Set WiFi to station mode and disconnect from an AP if it was previously connected.
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    Serial.println("Setup done");
}

void loop()
{
    Serial.println("Scan start");
    // WiFi.scanNetworks will return the number of networks found.
    int n = WiFi.scanNetworks();
    Serial.println("Scan done");
    if (n == 0) {
        Serial.println("no networks found");
    } else {
        Serial.println("start");
        for (int i = 0; i < n; ++i) {
            // Print SSID and RSSI for each network found
            Serial.printf("{\"MAC\":\"%s\",\"RSSI\":%d};\n", WiFi.BSSIDstr(i).c_str(), WiFi.RSSI(i));
            delay(10);
        }
    }
    Serial.println("end\n\r");
    // Delete the scan result to free memory for code below.
    WiFi.scanDelete();
    delay(5000);
}