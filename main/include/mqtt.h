#ifndef MESH_NETWORK_PROVISIONER_MQTT_H
#define MESH_NETWORK_PROVISIONER_MQTT_H

#include <mqtt_client.h>
#include "esp_log.h"
#include <cJSON.h>

#define MQTT_BROKER_URI "ws://fridaycloud.ddns.net:8001/mqtt"

static const esp_mqtt_client_config_t mqtt_cfg = {.uri = MQTT_BROKER_URI};

void mqtt_init();
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);
void mqtt_publish(float x, float y, float d0, float d1, float d2);


static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE("MQTT", "Last error %s: 0x%x", message, error_code);
    }
}
#endif //MESH_NETWORK_PROVISIONER_MQTT_H
