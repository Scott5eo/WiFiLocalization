#include "freertos/FreeRTOS.h"

#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "nvs_flash.h"

String input;
String macs[30];
int txs[30];
int i;
esp_err_t rsp;

const char ssid[32] = "";
const char password[32] = "brobro";
const uint8_t ssid_length = strlen(ssid);


void setup(){
    //setup serial if connected to computer
    Serial.begin(115200);

    esp_netif_init();
    esp_netif_create_default_wifi_ap();

    //init wifi block
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

    //init wifi config
    wifi_config_t wifi_config = {
        .ap = {
            .ssid = {0},
            .password = {0},
            .ssid_len = ssid_length,
            .channel = 1,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .ssid_hidden = 0,
            .max_connection = 4, 
            .beacon_interval = 60000
        }
    };

    //str copy ssid and password
    strcpy((char *)wifi_config.ap.ssid, ssid);
    strcpy((char *)wifi_config.ap.password, password);
    

    rsp = esp_wifi_init(&cfg);
    if (rsp != ESP_OK){
        Serial.println("Error initializing wifi");
        return;
    }

    //set wifi mode to AP
    rsp = esp_wifi_set_mode(WIFI_MODE_AP);
    if (rsp != ESP_OK){
        Serial.println("Error setting wifi mode");
        return;
    }

    rsp = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);

    //wait for config input
    while(Serial.available() == 0){
        delay(100);
        Serial.println("Waiting for input");
    }
    
    String input = Serial.readString();

    //manual parsing
    i=0;
    while(input.length() > 0){
        //get mac
        macs[i] = input.substring(0, input.indexOf(";")).c_str();
        Serial.println(macs[i]);
        input = input.substring(input.indexOf(";") + 1);

        //get tx
        txs[i] = input.substring(0, input.indexOf(",")).toInt();
        Serial.println(txs[i]);
        input = input.substring(input.indexOf(",") + 1);
        //increment i
        i++;
    }

    Serial.println("Setup done");
}

void loop(){
    for(int j = 0; j < i; j++){
        //for mac address macs[j], convert to uint8_t array
        Serial.print("Setting up mac address idx:");
        Serial.println(j);

        uint8_t mac[6];
        sscanf(macs[j].c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
        
        rsp = esp_wifi_set_mac(WIFI_IF_AP, mac);
        if (rsp != ESP_OK){
            Serial.println("Error setting mac address");
            return;
        }

        Serial.println("Setting up tx power");
        int8_t tx_power;
        switch(txs[j]){ //set tx power
            case 0:
                tx_power = 8;
                break;
            case 2:
                tx_power = 8;
                break;
            case 5:
                tx_power = 20;
                break;
            case 7:
                tx_power = 28;
                break;
            case 8:
                tx_power = 28;
                break;
            case 11:
                tx_power = 44;
                break;
            case 13:
                tx_power = 52;
                break;
            case 15:
                tx_power = 60;
                break;
            case 17:
                tx_power = 66;;
                break;
            case 18:
                tx_power = 72;
                break;
            case 19:
                tx_power = 80;
                break;
            case 20:
                tx_power = 80;
                break;
            //default to 80
            default:
                tx_power = 84;
                break;
        }
        //start softAP
        Serial.println("Starting softAP");
        esp_wifi_start();
        esp_wifi_set_ps(WIFI_PS_NONE);

        //set tx power
        ESP_ERROR_CHECK(esp_wifi_set_max_tx_power(tx_power));
        
        //spam packets
        //packet to spam
        uint8_t pkt[] = { 
                        0x0d, 0x00, //Frame Control version=0 type=0(management)  subtype=1101 (action) fromDS toDS set to 0
                        0x00, 0x00, //Duration
                /*4*/   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, //A1 Destination address  broadcast or NaN multicast (51-6f-9a-01-00-00) 
                /*10*/  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], //A2 Source address - 
                /*16*/  mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], //A3 BSSID 
                /*22*/  0x00, 0x00, //Seq-ctl
                /*24*/  0x00, 0x00, //Category
                /*26*/  0x04, 0x00, //Action code

        };

        for(int i = 0; i < 4; i++){
            ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_AP, pkt, sizeof(pkt),false));
            vTaskDelay(200/portTICK_PERIOD_MS);
        }

        //stop softAP
        Serial.println("Stopping softAP");
        ESP_ERROR_CHECK(esp_wifi_stop());
        vTaskDelay(200/portTICK_PERIOD_MS);
    }
}

