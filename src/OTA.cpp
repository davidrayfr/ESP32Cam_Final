// Programme de mise en place de l'OTA dans ESP avec lecture données WiFi dans EEPROM
#include <Arduino.h>
#include <ArduinoOTA.h>
#include "OTA.h"

void confOTA(char *hostname,char *ota_password){
  // Port defaults to 8266
  ArduinoOTA.setPort(3232);

  // Set Hostname
  ArduinoOTA.setHostname(hostname);

  // No authentication by default
  ArduinoOTA.setPassword(ota_password);

  ArduinoOTA.onStart([]() {
    Serial.println("Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}