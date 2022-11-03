// Gestion eeprom
#include <Arduino.h>
#include <EEPROM.h>

#define EEPROM_MEMORY_START 0
#define EEPROM_MEMORY_SIZE 512

/*
WiFi Mode
WIFI_OFF     WIFI_MODE_NULL
WIFI_STA     WIFI_MODE_STA
WIFI_AP      WIFI_MODE_AP
WIFI_AP_STA  WIFI_MODE_APSTA
*/
extern const unsigned long STRUCT_MAGIC;

struct EEPROM_Data {
  unsigned long magic;
  char WiFiMode[11];
  char ssid[32];
  char WiFiPassword[32];
  char OTApassword[32];
  char hostname[64];
  bool http_enable;
  bool rtsp_enable;
  unsigned short rtsp_port;
  };

void showEEPROM();

void cleanEEPROM();

void saveEEPROM(EEPROM_Data memory);

EEPROM_Data loadEEPROM(EEPROM_Data initial);