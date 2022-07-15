#ifndef MESH_NETWORK_PROVISIONER_IBEACON_MODEL_MESH_H
#define MESH_NETWORK_PROVISIONER_IBEACON_MODEL_MESH_H

#include "esp_ble_mesh_common_api.h"

#define ESP_BLE_MESH_IBEACON_MODEL_ID_SERVER      0x1416  /*!< Custom Server Model ID */
#define ESP_BLE_MESH_IBEACON_MODEL_ID_CLIENT      0x1417  /*!< Custom Client Model ID */

#define ESP_BLE_MESH_IBEACON_MODEL_OP_GET         ESP_BLE_MESH_MODEL_OP_3(0x03, CID_ESP)
#define ESP_BLE_MESH_IBEACON_MODEL_OP_BEACON         ESP_BLE_MESH_MODEL_OP_3(0x04, CID_ESP)
#define ESP_BLE_MESH_IBEACON_MODEL_OP_STATUS      ESP_BLE_MESH_MODEL_OP_3(0x05, CID_ESP)


typedef struct __attribute__((packed)){
    uint8_t uuid[16];
    uint16_t major;
    uint16_t minor;
    int rssi;
    double distance;
    int counter;
} model_ibeacon_data_t;

#endif //MESH_NETWORK_PROVISIONER_IBEACON_MODEL_MESH_H
