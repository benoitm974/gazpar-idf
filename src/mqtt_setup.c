#include "mqtt_setup.h"
#include "secrets.h"
#include "settings.h"

static const char *TAG = "MQTT";

/**
 * return the ret value of the publish message call, or -1 f any error
*/
int mqtt_message(unsigned int ticks){
    int ret = -1;
    char buf[20];

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

    sprintf(buf, "{ \"ticks\": %u }", ticks);
    ret = esp_mqtt_client_publish(client, CONFIG_BROKER_TOPIC, buf, 0, 1, 1);
    if (ret == -1 ) {
        ESP_LOGE(TAG, "Message not sent!" );
    } else {
        ESP_LOGI(TAG, "Message sent %s, return: %d", buf, ret);
    } 

    esp_mqtt_client_stop(client);

    return ret;
}


