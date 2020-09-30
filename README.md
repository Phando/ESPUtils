# ESPUtils
A library to make developing on ESP32 devices easy. Includes BLE Communitcation, LoRa Communication, OTA Updates, WiFi Management, a Salesforce connector\** and other general Utilities. The library allows you to pick and choose functionality without having to include all the source thus saving memory on your device. The ESPUtils library resides in the include directory, you will find the source folder is empty. 

\** The SFManager could be absrtracted to be an OAuth bearer connector at a later date

# Examples
All Examples are inteded for use in [PlatformIO](https://platformio.org/). You can see the collection of examples to help get you started in the [examples folder](examples)

# Library
## ESPUtils
This file is the heart of the library. By leveraging this class correctly, you will have all the features of this library at your fingertiips. All you need to do is call 
`ESPUtils::loop()` inside your main loop. By doing so, ESPUtils will call the `XXXManager::loop()` functions of the managers which have been selected for use.

## Config
This file contains selectors to include library managers as well as stubbed out versions of the parameters those managers require to operate correctly. If you require functionality outside the utilities provided by *ESPUtils.h*, copy *Config.h* from the lib directory to the root of your project. The two files should be included in this order.

```
#include <ESPUtils.h>
#include <Config.h>
```
At the top or *Config.h* you will find the following set of #define statements. By uncommenting these lines, the corresponding manager will be activated and a global instance variable will be made availabe with the exception of the WiFiManager which is a static class.

```
//#define USE_BLE   // BLEMan   - Bluetooth advertising, connections and communication
//#define USE_LORA  // LoRaMan  - LoRa communication
//#define USE_OTA   // OTAMan   - Over the Air updates
//#define USE_SFRA  // SFMan    - Salesforce Auth and Remote Access
//#define USE_WIFI  // Static   - Non blocking stable WiFi access
```
## BLEManager
## LoRaManager
## OTAanager
## SFManager
The Salesforce manager is your simple connector to getting and maintaining an OAuth token.
As mentioned at the top of the page, this class could be absrtracted to an OAuth bearer connector

## WiFiManager

# Helpers
## UtilCommon
## UtilMessage

# Partition Tables
An ESP32’s flash can contain multiple apps, as well as many different kinds of data (calibration data, filesystems, parameter storage, etc). How much memory is devoted to which use is configuraable through the use of partition tables. 

A set of alternative partition tables as well as additional infomration can be found in the [ESPUtils/partitions/README.md](partitions/README.md).



