/*
  ESPUtils.h - Library for easy development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#if !defined(ESP_UTILS_H)
#define ESP_UTILS_H

#include <UtilCommon.h>
#include <UtilConfig.h>
#include <UtilMessage.h>
#include <Preferences.h>
#include <esp_task_wdt.h>
#include <esp_wifi.h>

#ifdef USE_OTA
#include <OTAManager.h>
#endif

//---------------------------------------------------------------------

struct UtilButton {
  const uint8_t pin;
  bool dirty;
  bool pressed;
  uint32_t presses;

  UtilButton(uint8_t _pin) : pin(_pin), dirty(false), pressed(false), presses(0){};
  void toggle(){ pressed = !pressed; }
};

//---------------------------------------------------------------------

class ESPUtils {
public:
  static void clearPreferences(void);
  static int getParameterI(String key, int defaultValue = 0);
  static void setParameter(String key, int value);
  static String getParameterS(String key, String defaultValue = "");
  static void setParameter(String key, String value);
  static String getAddress();
  static String getShortAddress();
  static String getDeviceName();
  static void handleMessage(UtilMessage message);
  static void loop();
  static void restart();
  static void setClock();

private:
  
};

//---------------------------------------------------------------------

void ESPUtils::clearPreferences(void){
  static Preferences p;
  p.begin(UTIL_PREF_KEY, false);
  p.clear();
  p.end();
} 

//---------------------------------------------------------------------

String ESPUtils::getAddress() {
    uint8_t baseMac[6];
    esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
    char baseMacChr[18] = {0};
    sprintf(baseMacChr, "%02X%02X%02X%02X%02X%02X", baseMac[0], baseMac[1], baseMac[2], baseMac[3], baseMac[4], baseMac[5]);
    return String(baseMacChr);
}

//---------------------------------------------------------------------

String ESPUtils::getShortAddress(){
  uint8_t baseMac[6];
  esp_read_mac(baseMac, ESP_MAC_WIFI_STA);
  char baseMacChr[18] = {0};
  sprintf(baseMacChr, "%02X%02X", baseMac[4], baseMac[5]);
  return String(baseMacChr);
}

//---------------------------------------------------------------------

String ESPUtils::getDeviceName(){
  return UTIL_DEVICE_PREFIX + String("-")+ getShortAddress();
}

//---------------------------------------------------------------------

int ESPUtils::getParameterI(String key, int defaultValue) {
  static Preferences p;
  p.begin(UTIL_PREF_KEY, false);
  int value = p.getInt(key.c_str(), defaultValue);
  p.end();
  return value;
}

//---------------------------------------------------------------------

void ESPUtils::setParameter(String key, int value){
  static Preferences p;
  p.begin(UTIL_PREF_KEY, false);
  p.putInt(key.c_str(), value);
  p.end();
}

//---------------------------------------------------------------------

String ESPUtils::getParameterS(String key, String defaultValue) {
  static Preferences p;
  p.begin(UTIL_PREF_KEY, false);
  String value = p.getString(key.c_str(), defaultValue.c_str());
  p.end();
  return value;
}

//---------------------------------------------------------------------

void ESPUtils::setParameter(String key, String value){
  static Preferences p;
  p.begin(UTIL_PREF_KEY, false);
  p.putString(key.c_str(), value.c_str());
  p.end();
}

//---------------------------------------------------------------------
void ESPUtils::restart(){
  ESP.restart();
}

//---------------------------------------------------------------------

void ESPUtils::setClock() {
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

//---------------------------------------------------------------------

void ESPUtils::handleMessage(UtilMessage message){
  byte key = message.read();

  if(key == ESP_OTA_LOCAL){
    Serial.println("NFO: Repeatedly call ESPUtils::loop() to complete OTA.");
    otaManager = new OTAManager();
    otaManager->begin(handleMessage);
  } 
  else if(key == ESP_OTA_REMOTE){
    Serial.println("NFO: Repeatedly call ESPUtils::loop() to complete OTA.");
    otaManager = new OTAManager();
    otaManager->begin(handleMessage,message.readString());
  } 
  else if(key == ESP_SSID){
    setParameter(UTIL_SSID_KEY,message.readString());
  }
  else if(key == ESP_PASS){
    setParameter(UTIL_PASS_KEY,message.readString());
  }
  else if(key == ESP_LORA_REMOTE){
    setParameter(UTIL_REMOTE_ADDRESS,message.readString());
  }
  else if(key == ESP_RESTART){
    restart();
  }
}

//------------------------------------------------------------------------------------
void ESPUtils::loop(void) {
  
  // If OTA mode, handle the update
  if(otaManager != NULL){
    otaManager->loop();
    return;
  }

}

#endif