#if !defined(WIFI_MANAGER_H)
#define WIFI_MANAGER_H
/*
  WiFiManager.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#include <ESPUtils.h>
#include <HTTPClient.h>

#include <WiFi.h>
//#include <WiFiAP.h>
//#include <WiFiMulti.h>
#include <WiFiClientSecure.h>

//typedef void (*WiFiManagerCallback)();
enum UtilWifiState { idle, connecting, connected, disconnected };

//------------------------------------------------------------------------------------
class WiFiManager{
public:
    static unsigned long retryTime;
    static UtilWifiState state;
    static void setClock();
    static void beginAccessPoint();
    static void beginAccessPointAndStation();
    static void beginStation();
    static void beginConnection(const char* ssid, const char *pass, wifi_mode_t wifi_mode = WIFI_MODE_STA);
    static void beginConnection(wifi_mode_t wifi_mode = WIFI_MODE_STA);
    static void stopConnection();
    
    static void handleEvent(WiFiEvent_t event);
    static void onConnect();
    static void onDisconnect();
    static void loop();

private:
};

unsigned long WiFiManager::retryTime = 0;
UtilWifiState WiFiManager::state = idle;

//------------------------------------------------------------------------------------

void WiFiManager::handleEvent(system_event_id_t  event){
    switch(event) {
        case SYSTEM_EVENT_AP_START:
            WiFi.softAPsetHostname(AP_SSID);
            WiFi.softAPenableIpV6();
            break;

        case SYSTEM_EVENT_STA_START:
            WiFi.setHostname(AP_SSID);
            break;

        case SYSTEM_EVENT_STA_CONNECTED:
            WiFi.enableIpV6();
            break;

        case SYSTEM_EVENT_AP_STA_GOT_IP6:
            //both interfaces get the same event
            //Serial.println("STA IPv6: "+  String(WiFi.localIPv6()));
            //Serial.println("AP IPv6: "+ String(WiFi.softAPIPv6()));
            break;

        case SYSTEM_EVENT_STA_GOT_IP:
            WiFiManager::onConnect();
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            WiFiManager::onDisconnect();
            break;

        default:
            break;
    }
}

//------------------------------------------------------------------------------------

void WiFiManager::beginAccessPoint(){
    WiFi.softAP(AP_SSID);
    WiFiManager::beginConnection(WIFI_MODE_AP);
}
//------------------------------------------------------------------------------------

void WiFiManager::beginAccessPointAndStation(){
    WiFi.softAP(AP_SSID);
    WiFiManager::beginConnection(WIFI_MODE_APSTA);
}

//------------------------------------------------------------------------------------

void WiFiManager::beginStation(){
    WiFiManager::beginConnection(WIFI_MODE_STA);
}

//------------------------------------------------------------------------------------

void WiFiManager::beginConnection(const char* ssid, const char *pass, wifi_mode_t wifi_mode){
    WIFI_SSID = ssid;
    WIFI_PASS = pass;
    WiFiManager::beginConnection(wifi_mode);
}

//------------------------------------------------------------------------------------

void WiFiManager::beginConnection(wifi_mode_t wifi_mode){
    if(strcmp(WIFI_SSID,"CHANGE_ME") == 0 || strcmp(WIFI_PASS,"CHANGE_ME") == 0 ){
        Serial.println("ERROR: ESPUtils - Check to see WIFI_SSID and WIFI_PASS have been set in ESPUtils/Config.h or use WiFiManager::beginConnection(const char* ssid, const char *passphrase, wifi_mode_t wifi_mode)");
        return;
    }
    WiFi.onEvent(WiFiManager::handleEvent);
    WiFi.mode(wifi_mode);
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    WiFiManager::state = connecting;
}

//------------------------------------------------------------------------------------

void WiFiManager::stopConnection(){
    WiFiManager::state = idle;
    WiFi.disconnect();
} 

//------------------------------------------------------------------------------------

void WiFiManager::onConnect(){
    Serial.println("STA Connected");
    Serial.println("STA SSID: "+ WiFi.SSID());
    Serial.println("STA IPv4: "+ WiFi.localIP());
    Serial.print("STA IPv6: "+ WiFi.localIPv6().toString());
    WiFiManager::state = connected;
    WiFiManager::setClock();
}

//------------------------------------------------------------------------------------

void WiFiManager::onDisconnect(){
    Serial.println("STA Disconnected");
    if(WiFiManager::state == idle) {
        return;
    }
    WiFiManager::state = connecting;
    WiFiManager::retryTime = millis() + WIFI_RETRY_DELAY;
}

//---------------------------------------------------------------------

void WiFiManager::setClock() {
    // TODO : Check for connection
  Serial.println("Setting Clock");
  configTime(0, 0, CLOCK_SERVER_1, CLOCK_SERVER_2);

  Serial.print(F("TP time sync..."));
  time_t nowSecs = time(nullptr);
  while (nowSecs < 8 * 3600 * 2) {
    delay(500);
    Serial.print(F("."));
    yield();
    nowSecs = time(nullptr);
  }

  Serial.println();
  struct tm timeinfo;
  gmtime_r(&nowSecs, &timeinfo);
  Serial.print(F("Current time: "));
  Serial.print(asctime(&timeinfo));
}

//------------------------------------------------------------------------------------

void WiFiManager::loop(){
    if (WiFiManager::retryTime != 0 && WiFiManager::retryTime < millis()){
        WiFiManager::retryTime = 0;
        Serial.println("Attempting WiFi Reconnect");
        WiFiManager::beginConnection(WiFi.getMode());
    }
}

#endif