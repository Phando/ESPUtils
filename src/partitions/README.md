Partition Tables
These partition tables work for the ESP32 family of devices. 

In your platform.ini simply add a line to specify the parition table you want. If nothing is specified, the default.csv is used.

Default.csv


` 
Name,   Type, SubType, Offset,  Size, Flags  
nvs,      data, nvs,     0x9000,  0x5000,  
otadata,  data, ota,     0xe000,  0x2000,  
app0,     app,  ota_0,   0x10000, 0x140000,  
app1,     app,  ota_1,   0x150000,0x140000,  
spiffs,   data, spiffs,  0x290000,0x170000,  
`
