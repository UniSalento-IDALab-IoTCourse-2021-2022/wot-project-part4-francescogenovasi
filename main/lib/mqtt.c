
#include "mqtt.h"

esp_mqtt_client_handle_t client;

void mqtt_init(){
    ESP_LOGI("MQTT","Init MQTT");
    esp_err_t err;
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client,ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    err = esp_mqtt_client_start(client);
    if(err != ESP_OK){
        ESP_LOGE("MQTT","MQTT Start failed %s",mqtt_cfg.uri);
    } else ESP_LOGI("MQTT","Init complete!");
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    switch((esp_mqtt_event_id_t) event_id){
        case MQTT_EVENT_ANY:
            break;
        case MQTT_EVENT_ERROR:
            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                ESP_LOGI("MQTT", "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

            }
            break;
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI("MQTT","Connected to broker via WS");
            break;
        case MQTT_EVENT_DISCONNECTED:
            esp_mqtt_client_reconnect(client);
            break;
        case MQTT_EVENT_PUBLISHED:
            ESP_LOGI("MQTT", "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        default:
            break;
    }
}

void mqtt_publish(float x, float y, float d0, float d1, float d2){
    cJSON *data_object;
    data_object = cJSON_CreateObject();
    cJSON_AddNumberToObject(data_object,"x",x);
    cJSON_AddNumberToObject(data_object,"y",y);
    cJSON_AddNumberToObject(data_object, "r0", d0);
    cJSON_AddNumberToObject(data_object, "r1", d1);
    cJSON_AddNumberToObject(data_object, "r2", d2);
    char *str = cJSON_Print(data_object);
    esp_mqtt_client_publish(client,"BIMData_Architecture/Computer_Engineering_Cloud/IOT_Project/RTLS_MESH_NETWORK_IFC/Ecotekne/LaStecca/PrimoPiano/Idalab-002/Desk/uuid/0j9EEDh8jEev_8A15__li$",str,strlen(str),1,0);
    cJSON_free(data_object);
    cJSON_free(str);
}
