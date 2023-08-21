#include "mqtt_setup.h"
#include "secrets.h"
#include "settings.h"
#include "adc_bat.h"
#include <esp_wifi.h>

static const char *TAG = "MQTT";

int8_t getRSSI() {
    wifi_ap_record_t ap;
    esp_wifi_sta_get_ap_info(&ap);
    return  ap.rssi;
}

/**
 * return the ret value of the publish message call, or -1 f any error
*/
int mqtt_message(unsigned int ticks){
    int ret = -1;
    char buf[50];

    esp_mqtt_client_config_t mqtt_cfg = { 
        .broker.address.uri = CONFIG_BROKER_URL,
        //default.credentials.client_id = CONFIG_BROKER_USER,
        .credentials.authentication.password = CONFIG_BROKER_PASS,
        .credentials.username = CONFIG_BROKER_USER
    };
     esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    //esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);

    sprintf(buf, "{ \"ticks\": %u,  \"bat\": %d, \"rssi\": %d }", ticks, get_batt_voltage(), getRSSI());
    ret = esp_mqtt_client_publish(client, CONFIG_BROKER_TOPIC, buf, 0, 1, 1);
    if (ret == -1 ) {
        ESP_LOGE(TAG, "Message not sent!" );
    } else {
        ESP_LOGI(TAG, "Message sent %s, return: %d", buf, ret);
    } 

    esp_mqtt_client_stop(client);

    return ret;
}


