// Gestion eeprom
#include <Arduino.h>
#include <EEPROM.h>
#include "eeprom_Sauv.h"

#define EEPROM_MEMORY_START 0
#define EEPROM_MEMORY_SIZE 512

/*
WIFI_OFF     WIFI_MODE_NULL
WIFI_STA     WIFI_MODE_STA
WIFI_AP      WIFI_MODE_AP
WIFI_AP_STA  WIFI_MODE_APSTA */

const unsigned long STRUCT_MAGIC = 123456789;

const struct EEPROM_Data VIDE={
                    STRUCT_MAGIC,
                    "",
                    "",
                    "",
                    "",
                    "",
                    true,
                    true,
                    1
                };

/*struct EEPROM_Data {
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
*/
// Value initialisation for Strip
// Magic indicate if value already saved in EEPROM

void saveEEPROM(EEPROM_Data memory) {
  // Met à jour le nombre magic et le numéro de version avant l'écriture
  memory.magic = STRUCT_MAGIC;
  Serial.println("sauvegarde Memory > EEPROM");
  EEPROM.begin(EEPROM_MEMORY_SIZE);
  EEPROM.put(EEPROM_MEMORY_START, memory);
  EEPROM.commit();
  EEPROM.end();
}

EEPROM_Data loadEEPROM(EEPROM_Data initial) {
  EEPROM_Data EEPROM_memory;
  // Lit la mémoire EEPROM
  Serial.println("Load Data from EEPROM");
  EEPROM.begin(EEPROM_MEMORY_SIZE);
  EEPROM.get(EEPROM_MEMORY_START, EEPROM_memory);
  EEPROM.end();
  Serial.println("EEPROM Load done / Memory : "+(String(sizeof(EEPROM_memory))));
  // Détection d'une mémoire non initialisée
  if (EEPROM_memory.magic != initial.magic)
      {
      Serial.println("Magic NOK / EEPROM non initialisé");
      EEPROM_memory=initial;
      saveEEPROM(EEPROM_memory);
      }
      else
      {
      Serial.println("Recover EEPROM Data");
      }
  return EEPROM_memory;
  }

void showEEPROM()
{ 
  EEPROM_Data m;
  m=loadEEPROM(VIDE);
  Serial.println("Affichage eeprom");
  Serial.println(m.magic);
  Serial.println(m.WiFiMode);
  Serial.println(m.ssid);
  Serial.println(m.WiFiPassword);
  Serial.println(m.OTApassword);
  Serial.println(m.hostname);
  Serial.println(m.http_enable);
  Serial.println(m.rtsp_enable);
  Serial.println(m.rtsp_port);
}

void cleanEEPROM()
{
EEPROM.begin(EEPROM_MEMORY_SIZE);  
for (int i = 0; i < EEPROM_MEMORY_SIZE; i++) {
 EEPROM.write(EEPROM_MEMORY_START+i, 0);
 }
EEPROM.commit();
EEPROM.end();
}