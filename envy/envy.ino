#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <EEPROM.h>
#include "WifiCredentials.h"

#include <ArduinoMqttClient.h>

void writeString(char add,String data);
String read_String(char add);

/* EEPROM is used for storing device id and wifi credentials. Total size: 4k
layout
*/
#define EEPROM_DEV_ID_ADDR      0
#define EEPROM_WIFI_SSID_ADDR   64
#define EEPROM_WIFI_PW_ADDR     128


//#define DEVICE_ID "arbeitszimmer"
//#define DEVICE_ID "schlafen-kinder"
//#define DEVICE_ID "schlafen-eltern"
//#define DEVICE_ID "wc-gast"
//#define DEVICE_ID "wohnzimmer"
#define DEVICE_ID "unknown"
#define FW_VERSION "0.3.0"

// SHT30 temperature / humidity
Adafruit_SHT31 sht31 = Adafruit_SHT31();

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);
const char broker[] = "192.168.1.200";
int        port     = 1883;

String deviceID = "";

void setup() {
  Serial.begin(115200);
  Serial.printf("Booting v%s\n", FW_VERSION);

  EEPROM.begin(512);

  Serial.println("EEPROM started");

  // read from eeprom
  deviceID = readString(EEPROM_DEV_ID_ADDR);
  Serial.printf("device id from eeprom: ");
  Serial.println(deviceID);

  String ssid = readString(EEPROM_WIFI_SSID_ADDR);
  String password = readString(EEPROM_WIFI_PW_ADDR);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  checkOta(deviceID);

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);

  if (!mqttClient.connect(broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());

    while (1);
  }

  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  publishMqttStr("fwversion", FW_VERSION);

  // start sht30 
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
  }
  Serial.print("Heater Enabled State: ");
  if (sht31.isHeaterEnabled())
    Serial.println("ENABLED");
  else
    Serial.println("DISABLED");
}

void loop() {
  ArduinoOTA.handle();
  // call poll() regularly to allow the library to send MQTT keep alives which
  // avoids being disconnected by the broker
  mqttClient.poll();

  // Store measured value into point
  // Report RSSI of currently connected network
  publishMqtt("rssi", WiFi.RSSI());

  // add sht30 data
  float temperature = sht31.readTemperature();
  publishMqtt("temperature", temperature);
  publishMqtt("humidity", sht31.readHumidity());  

  //Wait 10s
  Serial.println("Wait 10s");
  delay(10000);
}

void publishMqtt(String name, float value) {
  Serial.printf("publish %s: %f\n", name.c_str(), value);

  String topic = "telemetry/arduino-" + deviceID + "/" + name;

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic.c_str());
  mqttClient.print(value);
  mqttClient.endMessage();
}

void publishMqttStr(String name, String value) {
  Serial.printf("publish %s: %s\n", name.c_str(), value.c_str());

  String topic = "telemetry/arduino-" + deviceID + "/" + name;

  // send message, the Print interface can be used to set the message contents
  mqttClient.beginMessage(topic.c_str());
  mqttClient.print(value);
  mqttClient.endMessage();
}

void checkOta(String deviceID){
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(deviceID.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
  Serial.println("Ready !!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void initEEPROM(){
  writeString(EEPROM_DEV_ID_ADDR, DEVICE_ID);
  // writeString(EEPROM_WIFI_SSID_ADDR, STASSID);
  // writeString(EEPROM_WIFI_PW_ADDR, STAPSK);
}

void writeString(char add,String data)
{
  int _size = data.length();
  int i;
  for(i=0;i<_size;i++)
  {
    EEPROM.write(add+i,data[i]);
  }
  EEPROM.write(add+_size,'\0');   //Add termination null character for String Data
  EEPROM.commit();
  delay(10);
}


String readString(char add)
{
  char data[100]; //Max 100 Bytes
  int len=0;
  unsigned char k;
  k=EEPROM.read(add);
  //Serial.printf("read char %c, %d\n", k,k);
  if(k == 255){
    Serial.println("EEPROM seems to be empty, start initializinng it");
    initEEPROM();
    k = EEPROM.read(add);
  }
  while(k != '\0' && len<500)   //Read until null character
  {    
    k=EEPROM.read(add+len);
    data[len]=k;
    len++;
  }
  data[len]='\0';
  return String(data);
}
