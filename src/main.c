/*
 * Sample deep sleep with wake stub for ESP32-c3
 * (May contain bits of code found online, no license info)
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "esp_sleep.h"
#include "esp_log.h"
#include "rom/rtc.h"
#include "soc/rtc.h"
#include "esp_timer.h"

#include "soc/timer_group_reg.h"
#include "soc/timer_periph.h"
#include "soc/rtc_cntl_reg.h"

#include "hal/rtc_cntl_ll.h" //TODO a lot of primitive for handling stub will be added in 5.1 to be updated in stub belows

// gpio
#include "rom/gpio.h"
#include "driver/rtc_io.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "secrets.h"
#include "settings.h"

#include "wifi_station.h"
#include "sntp_setup.h"
#include "mqtt_setup.h"

#define GPIO_INPUT_GET(gpio_no)     ((gpio_input_get()>>gpio_no)&BIT0)

/* Variable holding number of times ESP32 restarted since first boot.
 * It is placed into RTC memory using RTC_DATA_ATTR and
 * maintains its value when ESP32 wakes from deep sleep.
 */
RTC_DATA_ATTR static unsigned int boot_count = 0;
RTC_DATA_ATTR static unsigned int gazpar_ticks = 0;
#ifndef WAKETEST_USE_TIMER_DURATION
RTC_DATA_ATTR static uint64_t next_wakeup = 0;
#endif

static const char *TAG = "app_main";

/**
 * TIMER SECTION
*/
#ifdef WAKETEST_USE_TIMER

#ifdef WAKETEST_USE_TIMER_HOUR
int next_hour() {

    struct timeval tv_now;
    time_t now;
    struct tm *timeinfo;
    uint64_t rtcnow;

    // Get RTC calibration
    uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

    //get current time
    gettimeofday(&tv_now, NULL);
    // Get current RTC time in ticks
    rtcnow = rtc_cntl_ll_get_rtc_time();
    
    //convert to timeinfo
    now = tv_now.tv_sec;
    timeinfo = localtime(&now);

    // calculate seconds until next time
    int seconds_elapsed = timeinfo->tm_sec + timeinfo->tm_min * 60;
    int seconds_until_next_hour = 60 - seconds_elapsed;

    // Calculate sleep duration in microseconds
    int64_t sleep_duration = (int64_t)seconds_until_next_hour* S_TO_NS - (int64_t)DEEP_SLEEP_TIME_OVERHEAD_US;
    if (sleep_duration < 0)
        sleep_duration = 0;

    // Convert microseconds to RTC clock cycles
    int64_t rtc_count_delta = (sleep_duration << RTC_CLK_CAL_FRACT) / period;

    // Set wakeup time
    next_wakeup = rtcnow + (uint64_t)rtc_count_delta;

    ESP_LOGI(TAG, "Seconds before next hour wakeup : %d\n", seconds_until_next_hour);

    return seconds_until_next_hour;
}
#endif

#ifdef WAKETEST_USE_TIMER_MINUTE

int next_minute() {

    struct timeval tv_now;
    time_t now;
    struct tm *timeinfo;
    uint64_t rtcnow;

    // Get RTC calibration
    uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

    //get current time
    gettimeofday(&tv_now, NULL);
    // Get current RTC time in ticks
    
    rtcnow = rtc_cntl_ll_get_rtc_time();
    
    //convert to timeinfo
    now = tv_now.tv_sec;
    timeinfo = localtime(&now);

    // calculate seconds until next time
    int seconds_elapsed = timeinfo->tm_sec;
    int seconds_until_next_hour = 60 - seconds_elapsed;

    // Calculate sleep duration in microseconds
    int64_t sleep_duration = (int64_t)seconds_until_next_hour* S_TO_NS - (int64_t)DEEP_SLEEP_TIME_OVERHEAD_US;
    if (sleep_duration < 0)
        sleep_duration = 0;

    // Convert microseconds to RTC clock cycles
    int64_t rtc_count_delta = (sleep_duration << RTC_CLK_CAL_FRACT) / period;

    // Set wakeup time
    next_wakeup = rtcnow + (uint64_t)rtc_count_delta;

    ESP_LOGE(TAG, "Seconds before next minutes wakeup : %d\n", seconds_until_next_hour);

    return seconds_until_next_hour;
}

#endif

#ifdef WAKETEST_USE_TIMER_DURATION
RTC_IRAM_ATTR void set_deepsleep_timer(uint64_t duration_us)
{
    // Get RTC calibration
    uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);

    // Calculate sleep duration in microseconds
    int64_t sleep_duration = (int64_t)duration_us - (int64_t)DEEP_SLEEP_TIME_OVERHEAD_US;
    if (sleep_duration < 0)
        sleep_duration = 0;

    // Convert microseconds to RTC clock cycles
    int64_t rtc_count_delta = (sleep_duration << RTC_CLK_CAL_FRACT) / period;

    // Get current RTC time
    uint64_t now = rtc_cntl_ll_get_rtc_time();

    // Set wakeup time
    uint64_t future = now + (uint64_t)rtc_count_delta;

   rtc_cntl_ll_set_wakeup_timer(future);
}
#endif

#endif


/**
 * Wake_stub
*/
static void RTC_IRAM_ATTR wake_stub()
{
    uint64_t current=0;
    
    // Get current RTC time
    uint64_t start = rtc_cntl_ll_get_rtc_time();

    // Get RTC calibration
    uint32_t period = REG_READ(RTC_SLOW_CLK_CAL_REG);
    // Convert microseconds to RTC clock cycles
    int64_t rtc_count_delta = ( (uint64_t)GAZPAR_DEBOUNCE_MS * 1000L << RTC_CLK_CAL_FRACT) / period;

    esp_default_wake_deep_sleep();
    int wake_reason = rtc_cntl_ll_get_wakeup_cause();

    // to boot, return here.
    // to deep sleep again, continue below
    if (wake_reason == 8) //8 is timer wake reason from RTC register
        return;
    else //assume ticks ?
        gazpar_ticks++;

    //debounce
    do
    {
        // Get current RTC time
        current = rtc_cntl_ll_get_rtc_time();

        REG_WRITE(TIMG_WDTFEED_REG(0), 1); //feed Watchdog during debounce wait
    } while (current-start<rtc_count_delta);
   

#ifdef WAKETEST_USE_GPIO
    // wait for GPIO to go idle, else we keep entering the stub
    while (GPIO_INPUT_GET(WAKETEST_GPIO_NUM) == WAKETEST_GPIO_LEVEL)
    {
        REG_WRITE(TIMG_WDTFEED_REG(0), 1);
    }
#endif

#ifdef WAKETEST_USE_TIMER
    // set a new timer trigger value
    #ifdef WAKETEST_USE_TIMER_DURATION
    set_deepsleep_timer(WAKETEST_TIMER_INTERVAL_S * S_TO_NS);
    #else
    rtc_cntl_ll_set_wakeup_timer(next_wakeup);
    #endif
    // clear pending interrupts (if not, causes boot loop)
    WRITE_PERI_REG(RTC_CNTL_INT_CLR_REG, 0xFFFF);
#endif

    // required to stay in deep sleep
    WRITE_PERI_REG(RTC_ENTRY_ADDR_REG, (uint32_t)&wake_stub);
    set_rtc_memory_crc();

    // Go to sleep.
    CLEAR_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    SET_PERI_REG_MASK(RTC_CNTL_STATE0_REG, RTC_CNTL_SLEEP_EN);
    while (true);
}




/**
 * Main Application
*/
void app_main(void)
{
    ++boot_count;
    ESP_LOGI(TAG, "Boot count: %d", boot_count);
    ESP_LOGI(TAG, "app_main start wake cause: %d", esp_sleep_get_wakeup_cause());
    ESP_LOGI(TAG, "cpu freq: %lu", ets_get_cpu_frequency());
    //vTaskDelay(2000 / portTICK_PERIOD_MS);

    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    if (wifi_init_sta()) {

        //SNTP /TODO: check wifi ok, check if not ntp done yet etc...
        sntp_setup_init();
        
        //send MQTT //TODO: test return value
        if (mqtt_message(gazpar_ticks)> -1) //if message sent then nb_ticks reset.
            gazpar_ticks = 0;
    }

    //stop wifi
    esp_wifi_stop();

#ifdef WAKETEST_USE_GPIO
    // Wake up on with logic level
    const gpio_config_t config = {
        .pin_bit_mask = BIT(WAKETEST_GPIO_NUM),
        .mode = GPIO_MODE_INPUT,
    };
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(esp_deep_sleep_enable_gpio_wakeup(BIT(WAKETEST_GPIO_NUM), WAKETEST_GPIO_LEVEL));
#endif


    // Wake up with timer
    #ifdef WAKETEST_USE_TIMER_DURATION
    esp_sleep_enable_timer_wakeup( (int64_t)WAKETEST_TIMER_INTERVAL_S * S_TO_NS); 
    #endif
    
    #ifdef WAKETEST_USE_TIMER_HOUR
    esp_sleep_enable_timer_wakeup( (int64_t)next_hour() * S_TO_NS); 
    #endif

    #ifdef WAKETEST_USE_TIMER_MINUTE
    esp_sleep_enable_timer_wakeup( (int64_t)next_minute() * S_TO_NS);
    #endif



    // configure to use custom stub
    esp_set_deep_sleep_wake_stub(&wake_stub);

    // Enter deep sleep
    //TODO: debug ESP_ERROR_CHECK(esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON));
    esp_deep_sleep_start();
}