// WIFI settings
#define CONFIG_ESP_MAXIMUM_RETRY 4

//comment below to use DHCP
//#define WIFI_NO_DHCP
#ifdef WIFI_NO_DHCP
#define WIFI_STATIC_IP "192.168.1.5"
#define WIFI_STATIC_NETMASK "255.255.255.0"
#define WIFI_STATIC_GW "192.168.1.1"
#define WIFI_STATIC_DNS_PRIMARY "192.168.1.1"
#endif

// SNTP settings
#define CONFIG_SNTP_TIME_SERVER "pool.ntp.org"
#define CONFIG_SNTP_SYNC_WAIT_TIME 50
#define CONFIG_SNTP_SYNC_WAIT_RETRY 15
#define CONFIG_SNTP_SYNC_WAIT_FACTOR 30


// GPIO trigger settings
#define GAZPAR_DEBOUNCE_MS 240

// Comment out this line if you're using the internal RTC RC (150KHz) oscillator.
//#define USE_EXTERNAL_RTC_CRYSTAL
#ifdef USE_EXTERNAL_RTC_CRYSTAL
#define DEEP_SLEEP_TIME_OVERHEAD_US (650 + 100 * 240 / CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ)
#else
#define DEEP_SLEEP_TIME_OVERHEAD_US (250 + 100 * 240 / CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ) //TODO: check influence of changin plateform.io default freq here
#endif // USE_EXTERNAL_RTC_CRYSTAL

//WAKE UP settings
#define WAKETEST_USE_GPIO
#define WAKETEST_USE_TIMER

#ifdef WAKETEST_USE_GPIO
#define WAKETEST_GPIO_NUM   4
#define WAKETEST_GPIO_LEVEL ESP_GPIO_WAKEUP_GPIO_HIGH
#endif

#ifdef WAKETEST_USE_TIMER
//uncommet one of the below to decide if you want at the next hour, at the next minute(usefull for debug), or every INTERVAL_S define below
//#define WAKETEST_USE_TIMER_DURATION
//#define WAKETEST_USE_TIMER_HOUR
#define WAKETEST_USE_TIMER_MINUTE

#ifdef WAKETEST_USE_TIMER_DURATION
#define WAKETEST_TIMER_INTERVAL_S   10
#endif

#endif

#define S_TO_NS 1000000ULL