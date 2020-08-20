/*
  UtilCommon.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.
*/

#if !defined(UTIL_COMMON_H)
#define UTIL_COMMON_H

#define CMD_KEY 0xC0

#define NFO_KEY     0xF0
#define NFO_PING    0xF1
#define NFO_READY   0xF2
#define NFO_SUCCESS 0xF3
#define NFO_FAILURE 0xF4

#define ESP_KEY         0xE0
//#define ESP_DEVICE_ID   0xE1
#define ESP_SSID        0xE2
#define ESP_PASS        0xE3
#define ESP_BLE_ATTACH  0xE4
#define ESP_BLE_DETACH  0xE5
#define ESP_OTA_LOCAL   0xE6
#define ESP_OTA_REMOTE  0xE7
#define ESP_LORA_REMOTE 0xE8
#define ESP_RESTART     0xEF

#define UTIL_DEVICE_PREFIX "ESP" // Set this to something project relevant
#define UTIL_DEVICE_INIT   "ESP_INIT" // A one time flag to indicate a non initialized device id

// ESP Paramater keys 
#define UTIL_SSID_KEY "SSID"
#define UTIL_PASS_KEY "PASS"
#define UTIL_PREF_KEY "ESPUTILS"
#define UTIL_REMOTE_ADDRESS "REMOTE_ADDRESS"

#define ARRAY_SIZE(A) (sizeof(A)/sizeof((A)[0]))

#define IS_KEY_FOR(key, range) (key - range >= 0x00 && key - range <= 0x0F )
#define IS_KEY_CMD(key) (IS_KEY_FOR(key,CMD_KEY))
#define IS_KEY_ESP(key) (IS_KEY_FOR(key,ESP_KEY))
#define IS_KEY_NFO(key) (IS_KEY_FOR(key,NFO_KEY))

#endif