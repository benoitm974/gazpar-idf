# gazpar-idf

This is a ESP32 espressif-idf project to be run on TTGO T-OI plus (ESP32C3 - RISC-V), to count number of gaz 'gazpar' 'ticks' per period of time (every hours or every X minutes) or every X secondes/minutes and send it to an MQTT server with intend to be displayed in homeassistant, but any MQTT would work.

The goal was not to use the arduino layer but directly the espressif idf framework to have better control onver deep-sleep and power consumption to achieve 1 year data collection over a 16340 850MAh battery

The lowest option would have been to use the ULP microprocessor to count tick, but the use of the wakeup stub function allow to count ticks without launching the app which should be enough to collect data and send number of tick every hour to the MQTT server for a full year. Note that the biggest power consumption is the Data transmission over WiFi/MQTT.

## Installation / configuration

project was developed using platformIO and espressif framework
- clone the git to a folder
- create a secrets.h file with WiFI credentials
- check all the settings in settings.h (static vs DHCP, period of transmission etc ...)
- build
- there is an option in settings about disabling DHCP and setting static ip/dns/gw in order to reduce connection time and enhance battery lifetime too.

## secrets.h template
```
// WiFi
#define CONFIG_ESP_WIFI_SSID "XXX"
#define CONFIG_ESP_WIFI_PASSWORD "XX"

//MQTT
#define CONFIG_BROKER_URL "mqtt://XXX:1883"
#define CONFIG_BROKER_PASS "XXX"
#define CONFIG_BROKER_USER "XXX"
#define CONFIG_BROKER_TOPIC "gazpar"
```

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

## 3D case
remixed a case from thingverse to host the TTGO- T-OI ESP32C3 usb-c version : https://www.thingiverse.com/thing:6186441

## credits
Thanks to espressif github thread https://github.com/espressif/esp-idf/issues/8208 and @igrr for the stub code which is used here for GPIO/Timer wake stub from gthub: https://gist.github.com/igrr/54f7fbe0513ac14e1aea3fd7fbecfeab

## Todo
 
 - enable Flash encryption for secrets protection
 - add test cases
 - DONE in current branch idf-5.1 : the current stub code is verbose and use direct esp REGISTRY calls, since it compatible with espressif idf 5.0 in 5.1 macros and function were added to help with this in the hal/rtc_cntl_ll.h we'll need to update to this once platformio stable version for IDF will be 5.1

## License

[MIT](https://choosealicense.com/licenses/mit/)
