
#if defined(ARDUINO_ARCH_ESP8266)
#include <ESP8266WiFi.h>
#elif defined(ARDUINO_ARCH_ESP32)
#include "WiFi.h"
#endif

#include "src/WifiEspNow/src/WifiEspNowBroadcast.h"
#define WIFI_CHANNEL 1
#define MAX_PEERS 20

#define STATUS_LED 2
#define LEDON HIGH
#define LEDOFF LOW

void setup(){
    pinMode(STATUS_LED, OUTPUT);

    Serial.begin(115200); Serial.println();   // C3 needs USB CDC On boot "Enabled"!
    Serial.printf("Starting ESPnow demo\n");
    WiFi.persistent(false);
    bool wifiok = WifiEspNowBroadcast.begin("ESPNOW", 3);
    if (!wifiok) { 
        Serial.println("WifiEspNowBroadcast.begin() failed"); 
        ESP.restart(); 
    }
    Serial.print("MAC address of this node is ");
    Serial.println(WiFi.softAPmacAddress());
}

void loop() {
    WifiEspNowBroadcast.loop();             // EspNowBroadcast worker
    WifiEspNowPeerInfo peers[MAX_PEERS];    // Declare array peers[] for peer info
    int nPeers = WifiEspNow.listPeers(peers, MAX_PEERS);  // store peer info in peers[]
    Serial.printf("Number of peers: %i ",nPeers);
    for (int i = 0; i < nPeers; ++i) {
        Serial.printf(" %02X:%02X:%02X:%02X:%02X:%02X ", peers[i].mac[0], peers[i].mac[1],
                       peers[i].mac[2], peers[i].mac[3], peers[i].mac[4], peers[i].mac[5]); 
    }
    Serial.println();
  
    // if connected: short off, long on - if not: short on, long off 
    if( nPeers>0 ) { 
        digitalWrite(STATUS_LED, LEDOFF); 
        delay(50); 
        digitalWrite(STATUS_LED, LEDON);
        } 
    else { 
        digitalWrite(STATUS_LED, LEDON);  
        delay(50);  
        digitalWrite(STATUS_LED, LEDOFF);
    }
  delay(950);}