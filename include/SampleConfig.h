#if !defined(UTIL_CONFIG_H)
#define UTIL_CONFIG_H

/*
  Config.h - ESPUtils Library for development on the ESP32 Platform
  Created by Joe Andolina, April 1, 2020.
  Released into the public domain.

---------------------------------------------------------------------

The easiest way to configure ESPUtils is to copy SampleConfig.h to Config.h
and populate the new file with your project specific values. Be sure to 
uncomment the appropriate lines below to include specific functionality. 
The reason for this complexity is to save space on the microcontroller by 
only including the needed libraries. That said, the min_spiffs partition 
table helps give apps a little extra space.

---------------------------------------------------------------------*/

//#define USE_BLE   // BLEMan   - Bluetooth advertising, connections and communication
//#define USE_LORA  // LoRaMan  - LoRa communication
//#define USE_OTA   // OTAMan   - Over the Air updates
//#define USE_SFRA  // SFMan    - Salesforce Auth and Remote Access
//#define USE_WIFI  // Static   - Non blocking stable WiFi access

//---------------------------------------------------------------------

#ifdef USE_BLE // Bluetooth advertising, connections and communication
// The service ID is 5 characters short. These characters are used to 
// denote DeviceId and Client/Server mode
#define SERVICE_UUID    "ABCDEF00-1234-5678-ABCD-E50E24D"
#define BLE_RX_UUID     "ABCDEF01-1234-5678-ABCD-E50E24DCCA9E"
#define BLE_TX_UUID     "ABCDEF02-1234-5678-ABCD-E50E24DCCA9E"

#include <BLEManager.h>
#endif

//---------------------------------------------------------------------

#ifdef USE_LORA // LoRa communication
#include <LoRaManager.h>
#endif

//---------------------------------------------------------------------
#if defined(USE_OTA) || defined(USE_SFRA) // OTA and SF depend on WIFI
#define USE_WIFI
#endif

#ifdef USE_WIFI // Non blocking stable WiFi access
const char *WIFI_SSID = "CHANGE_ME";
const char *WIFI_PASS = "CHANGE_ME";
#define AP_SSID "CHANGE_ME_IF_YOU_ARE_HOSTING_AN_ACCESS_POINT"

#define WIFI_RETRY_DELAY 200
#define CLOCK_SERVER_1 "pool.ntp.org"
#define CLOCK_SERVER_2 "time.nist.gov"

#include <WiFiManager.h>
#endif

//---------------------------------------------------------------------

#ifdef USE_OTA // Over The Air updates
#include <OTAManager.h>
#endif

//---------------------------------------------------------------------

#ifdef USE_SFRA // Salesforce Remote Access
// To obtain your root certificate, log into your Salesforce org, then enter the following line on the command line
// openssl s_client -connect YOUR_ORG_DOMAIN.force.com:443 -showcerts
// Copy the cert shown under the 'DigiCert Global Root CA' line
const char* SFRA_CERT = \
"-----BEGIN CERTIFICATE-----\n" \
"...\n" \
"-----END CERTIFICATE-----\n";

#define SFRA_CLIENT_ID "YOUR_CLIENT_ID"
#define SFRA_CLIENT_SECRET "YOUR_CLIENT_SECRET"
#define SFRA_USER "YOUR_SF_ORG_USERNAME"
#define SFRA_PASS "YOUR_SF_ORG_PASS"

#define SFRA_RETRY_DELAY 500 // Time between requests
#define SFRA_REFRESH_DELAY 10000 // Refresh Token request time < 15 min
#define SFRA_API "v49.0"

#include <SFManager.h>
#endif

//---------------------------------------------------------------------

// Pinmap for the TTGO LoRa32 Oled
//#define CS  18  // GPIO18 -- SX1278's CS
//#define RST 14  // GPIO14 -- SX1278's RESET
//#define IRQ 26  // GPIO26 -- SX1278's IRQ(Interrupt Request)
//#define SDA 21  // GPIO? -- SSD1306's SDA
//#define SCL 22  // GPIO? -- SSD1306's SCL

#endif // UTIL_CONFIG_H