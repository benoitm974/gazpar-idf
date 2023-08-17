#include "sntp_setup.h"
#include "settings.h"

static const char *TAG = "sntp";

static void obtain_time(void)
{
    int retry = 0;
    sntp_sync_status_t sntp_status;

    //assume wifi in ok and accessible skip DHCP SNTP (see idf example if inerested in DHCP SNTP)
    ESP_LOGI(TAG, "Initializing and starting SNTP");
    
    sntp_setservername(0, CONFIG_SNTP_TIME_SERVER);
    sntp_set_sync_mode(SNTP_SYNC_MODE_IMMED);
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_init();

    sntp_status = esp_sntp_get_sync_status();
    while( sntp_status != SNTP_SYNC_STATUS_COMPLETED && retry < CONFIG_SNTP_SYNC_WAIT_RETRY ) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, CONFIG_SNTP_SYNC_WAIT_RETRY);
        retry++;
        vTaskDelay( CONFIG_SNTP_SYNC_WAIT_TIME+retry*CONFIG_SNTP_SYNC_WAIT_FACTOR / portTICK_PERIOD_MS);
        sntp_status = esp_sntp_get_sync_status();
    }

    esp_sntp_stop();
}

void sntp_setup_init(void)
{
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    // Is time set? If not, tm_year will be (1970 - 1900).
    if (timeinfo.tm_year < (2016 - 1900)) {
        ESP_LOGI(TAG, "Time is not set yet. Connecting to WiFi and getting time over NTP.");
        obtain_time();
    }

}

