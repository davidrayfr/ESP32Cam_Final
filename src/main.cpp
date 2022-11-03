// Web Server with SPIFFS

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "OV2640.h"
#include "SimStreamer.h"
#include "OV2640Streamer.h"
#include "CRtspSession.h"
#include "OTA.h"
#include "eeprom_Sauv.h"
#include "datakeys.h"
#include "OneButton.h"
#include "SPIFFS.h"
#include "ESPAsyncWebServer.h"
#include "OneButton.h"

const int led = 2;

// Data regarding LED
#define RED_LED_PIN 33 // LED rouge: GPIO 33
#define WHITE_LED_PIN 4 // LED blanche: GPIO 4 - ESP32 CAM
#define BUTTON_PIN 12 // Bouton Branché sur GPIO 12
#define canalPWM 7 // un canal PWM disponible
#define MAX_PWM 128 // Intensity Max Led
int ledBright=0;
const int AMPLITUDE_MAX_LED=200; // Amplitude Max LED 
const int COEF=20; // COEF Evolution Pulse 1 Tres lent 20 Rapide
int pulseFlag = false; // pulseFlag=flase -> Blink pulseFlag= True
int whiteLedStatus = false;

// Data Regarding Button Management
#define TIME_LONG_CLICK_DETECTION 5000 // Detection Tps Mini long clic in Millisecondes
#define TIME_LONG_CLICK_START 1000 // Detection start Long Click (milliseconds)
#define TIME_AFTER_LONG_CLICK 5000 // Detection second click after long clic (milliseconds)
#define TIME_BLINK 100 // Time - Frequency blink for Led in MilliSecond
unsigned long pressStartTime;
int longClickId = false; // Detection Long Click

// Data regarding WebPortal
#define TIME_CONFIG_PORTAL 600000 // Time of Portal config open (in millisecond)
bool http_Config_Portal_activ=false;
// Date for Soft AP
IPAddress apIP = IPAddress(192, 168, 1, 1);

int64_t previousMillis=0;

AsyncWebServer server(80);

EEPROM_Data memory;

// initialisation Camera
OV2640 cam;

WiFiServer rtspServer(554);
CStreamer *streamer;
CRtspSession *session;
WiFiClient client; // FIXME, support multiple clients

// declaration in advance
void http_Config_Portal_Start();
void http_Config_Portal_Closure();

// Setup a new OneButton on pin PIN_INPUT
// The 2. parameter activeLOW is true, because external wiring sets the button to LOW when pressed.
OneButton button(BUTTON_PIN, true,true);
hw_timer_t *My_timer=NULL;

void handleREINIT()
{
  saveEEPROM(INITIAL_VALUE);
  Serial.println("Reset EEPROM / Restart ESP32");
  delay(1000);
  ESP.restart();
}

void handleRESTART()
{
  Serial.println("Restart ESP32");
  delay(1000);
  ESP.restart();
}

void whiteLedPulse()
{
    //digitalWrite(WHITE_LED_PIN, !digitalRead(WHITE_LED_PIN));
  //ledcWrite(canalPWM, 5+((ledBright*10)%250));   //  LED blanche allumée (rapport cyclique 0,1%!)
  int j=ledBright*COEF;
  // Variation Led Blanche
  ledcWrite(canalPWM,((j/AMPLITUDE_MAX_LED)%2)*(AMPLITUDE_MAX_LED-j%AMPLITUDE_MAX_LED)+(((AMPLITUDE_MAX_LED+j)/AMPLITUDE_MAX_LED)%2)*(j%AMPLITUDE_MAX_LED));
  ledBright++;
  }

void whiteLedBlink()
{
    if (ledcRead(canalPWM)==0) {
              ledcWrite(canalPWM, MAX_PWM);   //  LED blanche éteinte (rapport cyclique 0%)
    }
    else {
    ledcWrite(canalPWM, 0);
    }
}

void redLedBlink()
{
    digitalWrite(RED_LED_PIN, !digitalRead(RED_LED_PIN));
}

void IRAM_ATTR checkTicks() {
  // include all buttons here to be checked
  button.tick(); // just call tick() to check the state.
}

void IRAM_ATTR onTimer() {
  // include all buttons here to be checked
  if (pulseFlag)
  {
      whiteLedPulse();
  }
  else
  {
     whiteLedBlink();
  }  
}

// DOuble clic detection
void doubleClick()
{
Serial.println("Double Click detected > RESTART");
// Blink launch
handleRESTART();
}

void simpleClick()
{
Serial.println("Simple Click detected");
// Verificaton si clic after long clic
Serial.println(millis()-previousMillis);
if (longClickId) {
// Si Simple CLick après long click (> Reinitialisation)
  if (millis()-previousMillis<TIME_AFTER_LONG_CLICK){
    Serial.println("HANDLE REINIT");
    handleREINIT();
    }
  else{
    longClickId=false;
    timerAlarmDisable(My_timer);
    ledcWrite(canalPWM, 0);   //  LED blanche éteinte (rapport cyclique 0%)
    }
  }
 else
  {
  if(http_Config_Portal_activ)    
    http_Config_Portal_Closure();
    else
    http_Config_Portal_Start();
  }
}

// Long press detected
// Wait Second short clic to start in the TIME
// LED Blink
void longClick()
{
Serial.println("Long Click");
longClickId=true;
pulseFlag=false;
timerAlarmEnable(My_timer);
//Wait second clic during TIME_AFTER_LONG_CLICK
previousMillis=millis();
http_Config_Portal_activ=false;
}

// this function will be called when the button was held down for 1 second or more.
void pressStart() {
  Serial.println("pressStart()");
  if (timerAlarmEnabled(My_timer)) {
    timerAlarmDisable(My_timer);
    longClickId=false;
    digitalWrite(RED_LED_PIN, HIGH);
    ledcWrite(canalPWM, 0);   //  LED blanche éteinte (rapport cyclique 0%)
  }
  pressStartTime = millis() - TIME_LONG_CLICK_START; // as set in setPressTicks()
} // pressStart()

// this function will be called when the button was released after a long hold.
void pressStop() {
  Serial.print("pressStop(");
  Serial.print(millis() - pressStartTime);
  Serial.println(") detected.");
  if ((millis() - pressStartTime)>TIME_LONG_CLICK_DETECTION)
    {
      Serial.println("long hold detected / More than TIME_LONG_CLICK_START");
    }
    else {
      longClick();
    }
  } // pressStop()

// This function start after long press following by short clic

//Construction chaine XML pour envoi page WEB
String XML_ConnectionData(void){
    String ch=String("<?xml version = \"1.0\" ?>")+String("<inputs>");
    ch=ch+String("<version>")+String(version)+String("</version>");
    ch=ch+String("<wifimode>")+String(memory.WiFiMode)+String("</wifimode>");
    if (String(memory.WiFiMode)=="WIFI_STA")
      ch=ch+String("<ipadresse>")+WiFi.localIP().toString()+String("</ipadresse>");
    if (String(memory.WiFiMode)=="WIFI_AP")
      ch=ch+String("<ipadresse>")+WiFi.softAPIP().toString()+String("</ipadresse>");
    ch=ch+String("<namessid>")+String(memory.ssid)+String("</namessid>"); 
    ch=ch+String("<hostname>")+String(memory.hostname)+String("</hostname>");
    if (memory.http_enable) 
      {ch=ch+String("<http_enable>")+"1"+String("</http_enable>");}
      else
      {ch=ch+String("<http_enable>")+"0"+String("</http_enable>");}
    if (memory.rtsp_enable)
      {ch=ch+String("<rtsp_enable>")+"1"+String("</rtsp_enable>");}
      else
      {ch=ch+String("<rtsp_enable>")+"0"+String("</rtsp_enable>");};
    ch=ch+String("<portrtsp>")+String(memory.rtsp_port)+String("</portrtsp>");
    ch=ch+String("</inputs>");
  return ch; 
};

void setup()
{
  //----------------------------------------------------Serial
  Serial.begin(115200);
  Serial.println("\n");

  //----------------------------------------------------GPIO
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);

  //----------------------------------------------------SPIFFS
  if(!SPIFFS.begin())
  {
    Serial.println("Erreur SPIFFS...");
    return;
  }

  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file)
  {
    Serial.print("File: ");
    Serial.println(file.name());
    file.close();
    file = root.openNextFile();
  }

//----------------------------EEPROM INITIALISATION & LOADING
// Si mémoire vide -> Enregistrement de la valeur initiale
  memory=loadEEPROM(INITIAL_VALUE);
  showEEPROM();
  Serial.println(memory.WiFiMode);
  //----------------------------------------------------WIFI
  if (String(memory.WiFiMode)=="WIFI_STA")
    {
      // CONFIG CLIENT WIFI
    WiFi.mode(WIFI_STA);
    WiFi.begin(memory.ssid, memory.WiFiPassword);
	  Serial.print("Tentative de connexion...");
	  while(WiFi.status() != WL_CONNECTED)
	    {
		  Serial.print(".");
		  delay(100);
	    }
	  Serial.println("\n");
	  Serial.println("Connexion etablie!");
    Serial.print("Adresse IP: ");
	  Serial.println(WiFi.localIP());
    }
    else
    {
      // CONFIG ACCESS POINT WIFI
    if (String(memory.WiFiMode)=="WIFI_AP")
      {
      WiFi.mode(WIFI_AP);
      WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      bool result = WiFi.softAP(memory.hostname,NULL,1,0,1);
      if (!result)
        {
        Serial.println("AP Config failed.");
        return;
        }
    else
        {
        Serial.println("AP Config Success.");
        Serial.print("AP MAC: ");
        Serial.println(WiFi.softAPmacAddress());
        }
      }
    else
      {
      Serial.println("ERROR - no WiFi Mode");
      //handleREINIT();
      }
    }
 /*use mdns for host name resolution*/
  if (!MDNS.begin(memory.hostname)) { //http://esp32.local
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
      }
    }
  Serial.print("mDNS responder started with ");
  Serial.print(memory.hostname);
  Serial.println(".local");

   //----------------------------------------------------OTA
  confOTA(memory.hostname,memory.OTApassword);

// Configuration LED
  Serial.println("ledcSetup(canalPWM, 5000, 12)");
  ledcSetup(canalPWM, 5000, 12); // canal = 7, frequence = 5000 Hz, resolution = 12 bit

// enable the standard led on pin 13.
  Serial.println("ledcAttachPin(WHITE_LED_PIN, 7)");
  ledcAttachPin(WHITE_LED_PIN, 7); // Signal PWM broche 4, canal 7.
  ledcWrite(canalPWM, 0);   //  LED blanche éteinte (rapport cyclique 0%) 
  
  //pinMode(WHITE_LED_PIN, OUTPUT); // sets the digital pin as output
  Serial.println("pinMode(RED_LED_PIN, OUTPUT)");
  pinMode(RED_LED_PIN, OUTPUT); // sets the digital pin as output
     
  // Initiate Led
  //digitalWrite(WHITE_LED_PIN, HIGH);
  Serial.println("Allumage LED RED");
  digitalWrite(RED_LED_PIN, HIGH); // LED Rouge Etainte

// Configuration Bouton
// link the doubleclick function to be called on a doubleclick event.
  button.attachDoubleClick(doubleClick);
  button.attachClick(simpleClick);
  button.setPressTicks(TIME_LONG_CLICK_START); // that is the time when LongPressStart is called
  button.attachLongPressStart(pressStart);
  button.attachLongPressStop(pressStop);
  Serial.println("fin configuration Bouton");

// initialisation Od timer interrupt
  My_timer=timerBegin(0,80,true);
  timerAttachInterrupt(My_timer,&onTimer,true);
  timerAlarmWrite(My_timer,TIME_BLINK*1000,true);
  Serial.println("fin configuration Timer");
  }

void http_Config_Portal_Start()
{
  previousMillis=millis();
  timerAlarmEnable(My_timer);
  pulseFlag=true;
  //----------------------------------------------------SERVER
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/index.html","text/html");
  });
  server.on("/w3.css", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/w3.css", "text/css");
  });
  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/script.js", "text/javascript");
  }); 
  server.on("/jquery-3.6.1.min.js", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS, "/jquery-3.6.1.min.js", "text/javascript");
  });
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    request->send(SPIFFS,"/favicon.ico","image/ico");
  });
server.on("/Camera_img.png", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  request->send(SPIFFS,"/Camera_img.png","image/png");
  });

server.on("/receiveData", HTTP_POST, [](AsyncWebServerRequest *request){
    Serial.println("received Data from Web Client");
    if(request->hasParam("hostname", true)){
      String hostname = request->getParam("hostname", true)->value();
      Serial.println(hostname);
      if (hostname!="") strcpy(memory.hostname,hostname.c_str());
    };
     if(request->hasParam("wifiname", true)){
      String wifiname = request->getParam("wifiname", true)->value();
      Serial.println(wifiname);
      if (wifiname!="") strcpy(memory.ssid,wifiname.c_str());
     };
      if(request->hasParam("wifipassword", true)){
      String wifipassword = request->getParam("wifipassword", true)->value();
      Serial.println(wifipassword);
      if (wifipassword!="") strcpy(memory.WiFiPassword,wifipassword.c_str());
      };
      if(request->hasParam("networktype", true)){
      String networktype = request->getParam("networktype", true)->value();
      Serial.println(networktype);
      if (networktype!="") strcpy(memory.WiFiMode,networktype.c_str());
      };
      if(request->hasParam("rtspenable", true)){
      String rtspenable = request->getParam("rtspenable", true)->value();
      Serial.println(rtspenable);
      if (rtspenable="true") memory.rtsp_enable=true;
      else memory.rtsp_enable=false;
      };
      if(request->hasParam("httpenable", true)){
      String httpenable = request->getParam("httpenable", true)->value();
      Serial.println(httpenable);
       if (httpenable="true") memory.http_enable=true;
      else memory.http_enable=false;
      };
      if(request->hasParam("portrtsp", true)){
      String portrtsp = request->getParam("portrtsp", true)->value();
      Serial.println(portrtsp);
      if (portrtsp!="") memory.rtsp_port = atoi(portrtsp.c_str());
      };
    // Enregistrement dans EEPROM
    request->send(204);
    saveEEPROM(memory);
    showEEPROM();
});

//Envoie les data de la connection en XML
//via la fonction chaine
  server.on("/getData", HTTP_GET, [](AsyncWebServerRequest *request)
  {
  Serial.println("envoi Data XML vers client");
  request->send(200,"text/xml",XML_ConnectionData());
  });

  server.on("/restart", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("RESTART BUTTON");
    request->send(200);
  });

  server.on("/save", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("SAVE BUTTON");
    request->send(200);
  });
  
  server.on("/reinit", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    Serial.println("REINIT BUTTON");
    request->send(200);
    handleREINIT();
  });

  server.on("/camera", HTTP_GET, [](AsyncWebServerRequest *request)
  { 
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    request->send(200,"text/html",response);
    while (1)
    {
        cam.run();
        request->send(200,"image/jpg", (char *)cam.getfb());
    }
  });

// Demarrage Serveur
  server.begin();
  Serial.println("Serveur actif!");
  previousMillis=millis();
  http_Config_Portal_activ=true;
}

void http_Config_Portal_Closure()
{
  http_Config_Portal_activ=false;
  timerAlarmDisable(My_timer);
  ledcWrite(canalPWM, 0);   //  LED blanche éteinte (rapport cyclique 0%) 
  Serial.println("time ended / http Portal Closure");
  delay(2000);
  server.end();
}
/*
void handle_jpg_stream(void)
{
    WiFiClient client = server.client();
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n\r\n";
    server.sendContent(response);

    while (1)
    {
        cam.run();
        if (!client.connected())
            break;
        response = "--frame\r\n";
        response += "Content-Type: image/jpeg\r\n\r\n";
        server.sendContent(response);

        client.write((char *)cam.getfb(), cam.getSize());
        server.sendContent("\r\n");
        if (!client.connected())
            break;
    }
}

void handle_jpg(void)
{
    WiFiClient client = server.client();

    cam.run();
    if (!client.connected())
    {
        return;
    }
    String response = "HTTP/1.1 200 OK\r\n";
    response += "Content-disposition: inline; filename=capture.jpg\r\n";
    response += "Content-type: image/jpeg\r\n\r\n";
    server.sendContent(response);
    client.write((char *)cam.getfb(), cam.getSize());
}
*/

void rtsp_Stream_Server()
{
// Max Frame
    uint32_t msecPerFrame = 100;
    static uint32_t lastimage = millis();

    // If we have an active client connection, just service that until gone
    // (FIXME - support multiple simultaneous clients)
    if(session) {
        session->handleRequests(0); // we don't use a timeout here,
        // instead we send only if we have new enough frames
        uint32_t now = millis();
        if(now > lastimage + msecPerFrame || now < lastimage) { // handle clock rollover
            session->broadcastCurrentFrame(now);
            lastimage = now;

            // check if we are overrunning our max frame rate
            now = millis();
            if(now > lastimage + msecPerFrame)
                printf("warning exceeding max frame rate of %d ms\n", now - lastimage);
        }

        if(session->m_stopped) {
            delete session;
            delete streamer;
            session = NULL;
            streamer = NULL;
        }
    }
    else {
        client = rtspServer.accept();

        if(client) {
            //streamer = new SimStreamer(&client, true);             // our streamer for UDP/TCP based RTP transport
            streamer = new OV2640Streamer(&client, cam);             // our streamer for UDP/TCP based RTP transport
            session = new CRtspSession(&client, streamer); // our threads RTSP session and state
        }
    }
}

void loop()
{
//ArduinoOTA.handle();
button.tick();

// Check if Config Portal open
if (((previousMillis+TIME_CONFIG_PORTAL)<millis()) and (http_Config_Portal_activ))
  http_Config_Portal_Closure();
  if (((previousMillis+TIME_AFTER_LONG_CLICK)<millis())and (longClickId))
  {
  longClickId=false;
  timerAlarmDisable(My_timer);
  ledcWrite(canalPWM, 0);   //  LED blanche éteinte (rapport cyclique 0%) 
  Serial.println("time ended / long clic not anymore valid");
  delay(1000);
  }
}
