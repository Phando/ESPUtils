# Partition Tables
An ESP32’s flash can contain multiple apps, as well as many different kinds of data (calibration data, filesystems, parameter storage, etc). How much memory is devoted to which use is configuraable through the use of partition tables.

Official partition table documentation is availabe on the [Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html) site.

## Defaults
Out of the box your Esp32 likely has the default.csv partition table installed. This is a pretty flexible configuration as it has space for [SPIFFS](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/storage/spiffs.html?highlight=spiffs) and [OTA](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/system/ota.html?highlight=ota). If you are not using those features and your program needs more space, then you may need to look into alternative specifications.

#### default.csv
```
Name,   Type, SubType, Offset,  Size, Flags, 
nvs,      data, nvs,     0x9000,  0x5000,
otadata,  data, ota,     0xe000,  0x2000,
app0,     app,  ota_0,   0x10000, 0x140000,
app1,     app,  ota_1,   0x150000,0x140000,
spiffs,   data, spiffs,  0x290000,0x170000,
```
## Alternatative Specifications
If the default configuration does not make sense for your application. There are several optional configurations availabe in the *ESPUtils/partitions* folder listed below. Additional partition table configurations can be found as part of the [espressif/arduino-esp32](https://github.com/espressif/arduino-esp32/tree/master/tools/partitions) repository. 
               
* default.csv             
* default_8MB.csv                
* default_16MB.csv        
* huge_app.csv 
* large_spiffs_16MB.csv              
* min_spiffs.csv
* minimal.csv      
* no_ota.csv

### Configuration
If nothing is specified, the default.csv is used. To specify one of the alternative parition table configurations

* Copy the specification you are intersted from ESPUtils/paritions
* To the root of your project
* Add the following line to your platform.ini (for  min_spiffs.csv)

#### platform.ini
```
board_build.partitions = min_spiffs.csv

;Alternately you could use a path
;board_build.partitions = ../ESPUtils/partitions/min_spiffs.csv
```

##Custom Tables
If your project has very specific needs, it is possible to specify your own partition table configuration. More information about partition table customization can be found on the [Espressif](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-guides/partition-tables.html#creating-custom-tables) site.

