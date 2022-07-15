#ifndef TEST_MESH_NETWORK_BLUETOOTH_MESH_H
#define TEST_MESH_NETWORK_BLUETOOTH_MESH_H

#include <esp_ble_mesh_ble_api.h>
#include <esp_ble_mesh_provisioning_api.h>
#include <esp_ble_mesh_config_model_api.h>
#include <esp_ble_mesh_generic_model_api.h>
#include <esp_ble_mesh_common_api.h>
#include <string.h>
#include <esp_bt_device.h>
#include <esp_ble_mesh_networking_api.h>
#include "ibeacon_model_mesh.h"
#include "scan.h"
#include "esp_ibeacon_api.h"
#include "triangulation.h"

#define CID_ESP 0x02E5
#define MSG_SEND_TTL        3
#define MSG_SEND_REL        false
#define MSG_TIMEOUT         0
#define MSG_ROLE            ROLE_PROVISIONER
#define COMP_DATA_PAGE_0    0x00
#define PROV_OWN_ADDR 0x0001
#define APP_KEY_IDX 0x0000
#define APP_KEY_OCTET 0x12

static uint8_t dev_uuid[16] = {0xdd, 0xdd};

static esp_ble_mesh_client_t config_client;

static esp_ble_mesh_cfg_srv_t config_server = {
        .relay = ESP_BLE_MESH_RELAY_DISABLED,
        .beacon = ESP_BLE_MESH_BEACON_ENABLED,
#if defined(CONFIG_BLE_MESH_FRIEND)
        .friend_state = ESP_BLE_MESH_FRIEND_ENABLED,
#else
        .friend_state = ESP_BLE_MESH_FRIEND_NOT_SUPPORTED,
#endif
#if defined(CONFIG_BLE_MESH_GATT_PROXY_SERVER)
        .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_ENABLED,
#else
        .gatt_proxy = ESP_BLE_MESH_GATT_PROXY_NOT_SUPPORTED,
#endif
        .default_ttl = 7,
        /* 3 transmissions with 20ms interval */
        .net_transmit = ESP_BLE_MESH_TRANSMIT(2, 20),
        .relay_retransmit = ESP_BLE_MESH_TRANSMIT(2, 20),
};

static esp_ble_mesh_model_t root_models[] = {
        ESP_BLE_MESH_MODEL_CFG_SRV(&config_server),
        ESP_BLE_MESH_MODEL_CFG_CLI(&config_client)
};

static struct esp_ble_mesh_key {
    uint16_t net_idx;
    uint16_t app_idx;
    uint8_t app_key[16];
}prov_key;

typedef struct{
    uint8_t uuid[16];
    uint16_t unicast;
    uint8_t elem_num;
}esp_ble_mesh_node_info_t;

static esp_ble_mesh_node_info_t nodes[CONFIG_BLE_MESH_MAX_PROV_NODES] = {
        [0 ... (CONFIG_BLE_MESH_MAX_PROV_NODES -1)] = {
                .unicast = ESP_BLE_MESH_ADDR_UNASSIGNED,
                .elem_num = 0
        }
};

static esp_ble_mesh_client_op_pair_t  ibeacon_model_op_pair[] = {{ESP_BLE_MESH_IBEACON_MODEL_OP_GET,ESP_BLE_MESH_IBEACON_MODEL_OP_STATUS}};
static esp_ble_mesh_model_op_t ibeacon_model_op[] = {ESP_BLE_MESH_MODEL_OP(ESP_BLE_MESH_IBEACON_MODEL_OP_STATUS,2),ESP_BLE_MESH_MODEL_OP_END};
static esp_ble_mesh_client_t ibeacon_model_client = {.op_pair=ibeacon_model_op_pair,.op_pair_size=ARRAY_SIZE(ibeacon_model_op_pair)};

static esp_ble_mesh_model_t custom_models[] = {
        ESP_BLE_MESH_VENDOR_MODEL(CID_ESP,ESP_BLE_MESH_IBEACON_MODEL_ID_CLIENT,ibeacon_model_op,NULL,&ibeacon_model_client)
};
static esp_ble_mesh_elem_t elements[] = {
        ESP_BLE_MESH_ELEMENT(0, root_models, custom_models)
};

int ble_mesh_init();
static void provisioning_callback(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param);
static esp_err_t prov_complete(int node_idx, const esp_ble_mesh_octet16_t uuid, uint16_t unicast, uint8_t elem_num, uint16_t net_idx);
static void config_client_callback(esp_ble_mesh_cfg_client_cb_event_t event, esp_ble_mesh_cfg_client_cb_param_t *param);
static void custom_ibeacon_client_callback(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t *param);
static void recv_unprov_adv_pkt(uint8_t dev_uuid[16], uint8_t addr[BD_ADDR_LEN], esp_ble_mesh_addr_type_t addrType, uint16_t oob_info, uint8_t adv_type, esp_ble_mesh_prov_bearer_t bearer);
esp_err_t ble_mesh_ibeacon_model_client_message_get(void);
static esp_ble_mesh_node_info_t *ble_mesh_get_node_info(uint16_t unicast_addr);
static esp_err_t ble_mesh_set_msg_common(esp_ble_mesh_client_common_param_t *common, esp_ble_mesh_node_info_t *node, esp_ble_mesh_model_t *model, uint32_t opcode);
uint8_t * get_uuid_from_addr(uint16_t addr);

static esp_ble_mesh_prov_t provision = {
        .prov_uuid = dev_uuid,
        .prov_unicast_addr = PROV_OWN_ADDR,
        .prov_start_address = 0x0005,
        .prov_attention = 0x00,
        .prov_algorithm = 0x00,
        .prov_pub_key_oob = 0x00,
        .prov_static_oob_val = NULL,
        .prov_static_oob_len = 0x00,
        .flags = 0x00,
        .iv_index = 0x00
};

static esp_ble_mesh_comp_t composition = {
        .cid = CID_ESP,
        .elements = elements,
        .element_count = ARRAY_SIZE(elements)
};

void ble_mesh_get_dev_uuid(uint8_t *dev_uuid);

#endif //TEST_MESH_NETWORK_BLUETOOTH_MESH_H
