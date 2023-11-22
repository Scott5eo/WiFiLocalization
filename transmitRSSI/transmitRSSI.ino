#include "WiFi.h"

extern "C"{
    #include "esp_wifi.h"
    esp_err_t esp_wifi_set_channel(uint8_t primary, wifi_second_chan_t second);
    esp_err_t esp_wifi_80211_tx(wifi_interface_t ifx, const void *buffer, int len, bool en_sys_seq);
    esp_err_t esp_wifi_set_max_tx_power(int8_t power);
}

//run time variables
char emptySSID[32];
const uint8_t ssid_len = 32;
uint8_t mac[6];
uint8_t channel = 1;
uint32_t packetSize = 0;
uint32_t packetCount = 0;
uint32_t currentTime = 0;
uint32_t attackTime = 0;
String macs[30];
int txs[30];
int i = 0;

// beacon frame definition
uint8_t beaconPacket[109] = {
  /*  0 - 3  */ 0x80, 0x00, 0x00, 0x00, // Type/Subtype: managment beacon frame
  /*  4 - 9  */ 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, // Destination: broadcast
  /* 10 - 15 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // Source Address
  /* 16 - 21 */ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, // BSS ID
  /* 22 - 23 */ 0x00, 0x00, // sequence control

  //Frame body
  /* 24 - 31 */ 0x83, 0x51, 0xf7, 0x8f, 0x0f, 0x00, 0x00, 0x00, // Timestamp
  /* 32 - 33 */ 0x64, 0x00, // Interval: 0x64, 0x00 => every 100ms - 0xe8, 0x03 => every 1s
  /* 34 - 35 */ 0x31, 0x00, // capabilities Information

  // SSID parameters
  /* 36 - 37 */ 0x00, 0x20, // Tag: Set SSID length, Tag length: 32
  /* 38 - 69 */ 0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20,
                0x20, 0x20, 0x20, 0x20, // SSID 32 bytes

  // Supported Rates
  /* 70 - 71 */ 0x01, 0x08, // Tag: Supported Rates, Tag length: 8
  /* 72 */ 0x82, // 1(B)
  /* 73 */ 0x84, // 2(B)
  /* 74 */ 0x8b, // 5.5(B)
  /* 75 */ 0x96, // 11(B)
  /* 76 */ 0x24, // 18
  /* 77 */ 0x30, // 24
  /* 78 */ 0x48, // 36
  /* 79 */ 0x6c, // 54

  // Current Channel
  /* 80 - 81 */ 0x03, 0x01, // Channel set, length
  /* 82 */      0x01,       // Current Channel

  // RSN information
  /*  83 -  84 */ 0x30, 0x18,
  /*  85 -  86 */ 0x01, 0x00,
  /*  87 -  90 */ 0x00, 0x0f, 0xac, 0x02,
  /*  91 -  92 */ 0x02, 0x00,
  /*  93 - 100 */ 0x00, 0x0f, 0xac, 0x04, 0x00, 0x0f, 0xac, 0x04, /*Fix: changed 0x02(TKIP) to 0x04(CCMP) is default. WPA2 with TKIP not supported by many devices*/
  /* 101 - 102 */ 0x01, 0x00,
  /* 103 - 106 */ 0x00, 0x0f, 0xac, 0x02,
  /* 107 - 108 */ 0x00, 0x00
};


void setup(){
    //**********WIFI SETUP**********//
    //WiFi init, let the WiFi library handle it
    WiFi.mode(WIFI_MODE_STA);
    
    //generate empty SSID
    for(int i = 0; i < 32; i++){
        emptySSID[i] = ' ';
    }
    //get packet size
    packetSize = sizeof(beaconPacket);

    //**********CONFIG SETUP**********//
    String input;
    //Serial comms init
    Serial.begin(115200);

    //wait for config input
    while(Serial.available() == 0){
        delay(500);
        Serial.println("Waiting for input");
    }
    input = Serial.readString();
    //manual parsing
    while(input.length() > 0){
        //get mac
        macs[i] = input.substring(0, input.indexOf(";"));
        Serial.println(macs[i]);
        input = input.substring(input.indexOf(";") + 1);

        //get tx
        txs[i] = input.substring(0, input.indexOf(",")).toInt();
        Serial.println(txs[i]);
        input = input.substring(input.indexOf(",") + 1);
        //increment i
        i++;
    }
    int j = 0;
    while(j < i){
        Serial.println(macs[j]);
        j++;
    }
    Serial.println("Setup complete");
}

void loop(){
    for(int j=0; j<i; j++){
        currentTime = millis();
        //best effort basis for now
        nextChannel();

        //*******packet manipulation*******//
        //set mac address
        sscanf(macs[j].c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        memcpy(&beaconPacket[10], mac, 6);
        memcpy(&beaconPacket[16], mac, 6);

        //reset SSID
        memcpy(&beaconPacket[38], emptySSID, ssid_len);

        //change ssid to macs[j]
        const char* tmp = macs[j].c_str();
        memcpy(&beaconPacket[38], tmp, strlen(tmp));

        //change channel
        beaconPacket[82] = channel;

        //set power
        setPower(txs[j]);

        //send beacon
        ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_STA, beaconPacket, packetSize, false));
        vTaskDelay(100 / portTICK_PERIOD_MS);
        //increment packet count
        packetCount++;
        //increment attack time
        attackTime = millis();
    }
    Serial.print("Attack complete. Time taken:");
    Serial.println(attackTime - currentTime);
    Serial.print("Packets sent: ");
    Serial.println(packetCount);
}

void nextChannel(){
    //increment channel
    channel++;
    //check if channel is valid
    if(channel > 14){
        channel = 1;
    }
    //set channel
    Serial.print("Setting channel to: ");
    Serial.print(channel);
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    //vTask delay is 100 enough? idk
    vTaskDelay(100 / portTICK_PERIOD_MS);
    //read channel back
    wifi_second_chan_t second; //not used
    esp_wifi_get_channel(&channel, &second);
    Serial.print("Channel set to: ")
    Serial.println(channel);
}

void setPower(uint8_t tx_power){
    Serial.print("Setting power to: ");
    Serial.println(tx_power);
    ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(tx_power));
    vTaskDelay(10 / portTICK_PERIOD_MS)
    //read the power back
    int8_t power;
    esp_wifi_get_max_tx_power(&power);
    Serial.print("Power set to: ");
    Serial.println(power);
}