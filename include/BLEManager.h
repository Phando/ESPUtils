#if !defined(BLE_MANAGER_H)
#define BLE_MANAGER_H
/*
  BLEManager.h - Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#include <ESPUtils.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

//---------------------------------------------------------------------

class ServerCallbacks: public BLEServerCallbacks {
public:
    ServerCallbacks(UtilMessageCallback callback){
        _callback = callback;
    }

    void onConnect(BLEServer* pServer) {
      Serial.println("Attach");
      _callback({ESP_BLE_ATTACH});
    }

    void onDisconnect(BLEServer* pServer) {
        Serial.println("Detach");
      _callback({ESP_BLE_DETACH});
    }

private:
    UtilMessageCallback _callback;
};

//---------------------------------------------------------------------

class CharacteristicCallbacks: public BLECharacteristicCallbacks {
public:
    
    CharacteristicCallbacks(UtilMessageCallback callback){
        _callback = callback;
    }

    void onWrite(BLECharacteristic *rxCharacteristic) {
      string rxValue = rxCharacteristic->getValue();
      _callback(UtilMessage(rxValue));
    }

private:
    UtilMessageCallback _callback;
};

//---------------------------------------------------------------------

class BLEManager {
public:
    bool isServer = false;
    bool connected;
    UtilMessageCallback callback;
    BLECharacteristic *rxCharacteristic;
    BLECharacteristic *txCharacteristic;

    BLEManager(){};
    void beginClient(UtilMessageCallback callback);
    void beginServer(UtilMessageCallback callback);
    void sendMessage(string message);
    void sendMessage(UtilMessage message);
    static void handleDeviceCallback(UtilMessage message);
    
private:
    long refreshTime = 0;
    BLEServer *pServer;
    BLEService *pService;
    
    void begin(bool isServer, UtilMessageCallback callback);
};

BLEManager BLEMan;

//------------------------------------------------------------------------------------

void BLEManager::beginClient(UtilMessageCallback callback){
    begin(false, callback);
}

//------------------------------------------------------------------------------------

void BLEManager::beginServer(UtilMessageCallback callback){
    begin(true, callback);
}

//------------------------------------------------------------------------------------
void BLEManager::begin(bool isServer, UtilMessageCallback callback) {
    BLEMan.isServer = isServer;
    BLEMan.callback = callback;

    BLEDevice::init(ESPUtils::getDeviceName().c_str()); // 10 characters or less
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerCallbacks(handleDeviceCallback));

    String serviceTag = SERVICE_UUID + String(isServer) + ESPUtils::getShortAddress();
    
    BLEUUID serviceId = BLEUUID::fromString(serviceTag.c_str());
    pService = pServer->createService(serviceId);
    rxCharacteristic = pService->createCharacteristic(
        BLE_RX_UUID,
        BLECharacteristic::PROPERTY_NOTIFY);
                        
    rxCharacteristic->addDescriptor(new BLE2902());
    
    txCharacteristic = pService->createCharacteristic(
        BLE_TX_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    txCharacteristic->setCallbacks(new CharacteristicCallbacks(handleDeviceCallback));
    pService->start();
    
    Serial.println("UUID: " + String(pService->getUUID().toString().c_str()));
    pServer->getAdvertising()->addServiceUUID(pService->getUUID());  // needed for RB compat.
    pServer->getAdvertising()->start();
    
    Serial.println("Started Device: " + ESPUtils::getDeviceName() +" as " + isServer ? "server." : "client.");
    Serial.println("ServiceID: " + serviceTag);
    Serial.println("BLE - Waiting for connection...");
}

//------------------------------------------------------------------------------------

void BLEManager::sendMessage(string message) {
    txCharacteristic->setValue(message);
    txCharacteristic->notify();
}

//------------------------------------------------------------------------------------

void BLEManager::sendMessage(UtilMessage message) {
    sendMessage(message.readCString());
}

//------------------------------------------------------------------------------------

void BLEManager::handleDeviceCallback(UtilMessage message) {
    uint8_t key = message.front();
    
    if(key == ESP_BLE_ATTACH){
        Serial.println("BLE - Connected");
        BLEMan.connected = true;
    }
    else if(key == ESP_BLE_DETACH){
        Serial.println("BLE - Disconnected");
        BLEMan.connected = false;
    } 
    else {
        Serial.println("BLE - Pass");
        BLEMan.callback(message);
    }
}

#endif