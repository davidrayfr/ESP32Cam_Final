// copy this file to wifikeys.h and edit

const char *ssid = "MaisonRay300";
const char *password = "CamilleEmilie";
//const char *ssid = "Vatan2.4";
//const char *password = "Vascjbb5";

const char *version= "Version 0.2";
/*
WiFi Mode
WIFI_OFF     WIFI_MODE_NULL
WIFI_STA     WIFI_MODE_STA
WIFI_AP      WIFI_MODE_AP
WIFI_AP_STA  WIFI_MODE_APSTA
*/

//Initial Valeur stored in EEPROM
const struct EEPROM_Data INITIAL_VALUE={
                    STRUCT_MAGIC,
                    "WIFI_AP",
                    "MaisonRay300",
                    "CamilleEmilie",
                    "123456",
                    "Esp32Cam",
                    true,
                    true,
                    554
                };

