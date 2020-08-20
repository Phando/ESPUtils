/*
  LoRaManager.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#if !defined(LORA_MANAGER_H)
#define LORA_MANAGER_H

#include <LoRa.h>
#include <SPI.h>
#include <ESPUtils.h>

// TODO : Rename and move to Config.h
#define PING_INTERVAL 10000

//------------------------------------------------------------------------------------
class LoRaManager{
public:
  bool isServer = true;
  unsigned long lastPing;
  uint8_t localAddress[2];
  uint8_t remoteAddress[2];
  UtilMessage loraMessage;
  UtilMessageCallback callback;

  LoRaManager(){};
  void initAddresses();
  void beginClient(UtilMessageCallback callback);
  void beginServer(UtilMessageCallback callback);
  
  static void onReceive(int packetSize);
  bool connected();
  void sendMessage(byte data);
  void sendMessage(string message);
  void sendMessage(UtilMessage message);
  void loop();

private:
  bool sendResponse = false;
  void beginAs(bool isServer, UtilMessageCallback callback);
  void handleMessage();
  void readMessage();
  void rxMode();
  void txMode();
};

LoRaManager LoRaMan;

//------------------------------------------------------------------------------------
void LoRaManager::beginClient(UtilMessageCallback callback){
  beginAs(false, callback);
}

//------------------------------------------------------------------------------------
void LoRaManager::beginServer(UtilMessageCallback callback){
  beginAs(true, callback);
}

//------------------------------------------------------------------------------------
void LoRaManager::initAddresses(){
  String mac = ESPUtils::getShortAddress();
  LoRaMan.localAddress[0] = strtol(mac.substring(0,2).c_str(), NULL, 16);
  LoRaMan.localAddress[1] = strtol(mac.substring(2,4).c_str(), NULL, 16);

  mac = ESPUtils::getParameterS(UTIL_REMOTE_ADDRESS, "0000");
  LoRaMan.remoteAddress[0] = strtol(mac.substring(0,2).c_str(), NULL, 16);
  LoRaMan.remoteAddress[1] = strtol(mac.substring(2,4).c_str(), NULL, 16);
}

//------------------------------------------------------------------------------------
void LoRaManager::beginAs(bool isServer, UtilMessageCallback callback){
  LoRaMan.isServer = isServer;
  LoRaMan.callback = callback;
  LoRa.setPins(CS, RST, IRQ);// set CS, reset, IRQ pin

  if (!LoRa.begin(915E6)) {             // initialize ratio at 915 MHz
    Serial.println("LoRa init failed. Check your connections.");
    while (true); // if failed, do nothing
  }
  
  LoRaMan.initAddresses();
  LoRa.setTxPower(20);
  LoRa.onReceive(onReceive);
  Serial.println("LoRa init succeeded.");
  rxMode();
}

//------------------------------------------------------------------------------------

bool LoRaManager::connected(){
  return lastPing + (PING_INTERVAL*3) > millis();
}

//------------------------------------------------------------------------------------

void LoRaManager::sendMessage(byte data) {
  sendMessage(UtilMessage(data));
}

//------------------------------------------------------------------------------------

void LoRaManager::sendMessage(string message) {
  sendMessage(UtilMessage(message));
}

//------------------------------------------------------------------------------------

void LoRaManager::sendMessage(UtilMessage message) {
  txMode();
  LoRa.beginPacket();              
  LoRa.write(localAddress,2);
  LoRa.write(remoteAddress,2);            
  LoRa.write(message.size());
  while(message.bytesAvailable()){
    LoRa.write(message.read());
  }
  LoRa.endPacket();
  rxMode();
}

//------------------------------------------------------------------------------------
void LoRaManager::onReceive(int packetSize) {
  byte buffer[2];

  LoRa.readBytes(buffer,2);
  if (buffer[0] != LoRaMan.remoteAddress[0] || buffer[1] != LoRaMan.remoteAddress[1]){
    Serial.println("Ignore: unknown sender.");
    return;
  }

  LoRa.readBytes(buffer,2); 
  if (buffer[0] != LoRaMan.localAddress[0] || buffer[1] != LoRaMan.localAddress[1]){
    Serial.println("Ignore: wrong address.");
    return;
  }

  // Read payload
  byte expectedLength = LoRa.read(); // incoming msg length
  LoRaMan.loraMessage.clear();
  while (LoRa.available()) {
    LoRaMan.loraMessage.write(LoRa.read());
  }

  // Valid payload?
  if (expectedLength != LoRaMan.loraMessage.bytesAvailable()) {
    Serial.println("Ignore: size.");
    LoRaMan.loraMessage.clear();
    return;
  }
}

//------------------------------------------------------------------------------------
void LoRaManager::handleMessage() {
  callback(loraMessage);
  loraMessage.clear();
  if(!isServer){
    sendMessage(NFO_PING);
  }
}

//------------------------------------------------------------------------------------
void LoRaManager::rxMode(){
  //Serial.println("RX Mode");
  isServer ? LoRa.disableInvertIQ() : LoRa.enableInvertIQ();
  LoRa.receive();        
}

//------------------------------------------------------------------------------------
void LoRaManager::txMode(){
  //Serial.println("TX Mode");
  LoRa.idle();
  isServer ? LoRa.enableInvertIQ() : LoRa.disableInvertIQ();
}

//------------------------------------------------------------------------------------
void LoRaManager::loop(){
  if(loraMessage.available()){
    handleMessage();
  }
}

#endif
