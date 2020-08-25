#if !defined(ESP_UTILS_H)
#define ESP_UTILS_H
/*---------------------------------------------------------------------

 ESPUtils.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.

---------------------------------------------------------------------

The easiest way to configure ESPUtils is to copy Config.h to your project
and populate the new file with your project specific values. Be sure to 
uncomment the appropriate lines below to include specific functionality. 
The reason for this complexity is to save space on the microcontroller by 
only including the needed libraries. That said, the min_spiffs partition 
table helps give apps a little extra space. More information can be found 
in the README.md and in the Config.h file.

---------------------------------------------------------------------*/

#include <UtilCommon.h>
#include <UtilMessage.h>
#include <Preferences.h>

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

private:
  
};

#include <Config.h>


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

void ESPUtils::handleMessage(UtilMessage message){
  byte key = message.read();

  #ifdef USE_OTA
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
  else
  #endif 
  if(key == ESP_SSID){
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

  // Maintain the WiFi connection
  #ifdef USE_WIFI
  WiFiManager::loop();
  #endif

  // If OTA mode, handle the update
  #ifdef USE_OTA 
  if(OTAMan != NULL){
    OTAMan->loop();
    return;
  }
  #endif

  #ifdef USE_LORA
  LoRaMan.loop();
  #endif

  #ifdef USE_SFRA
  SFMan.loop();
  #endif
  yield();
}

#endif