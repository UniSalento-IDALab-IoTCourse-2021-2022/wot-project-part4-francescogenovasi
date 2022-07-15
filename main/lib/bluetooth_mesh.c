#include "bluetooth_mesh.h"


esp_err_t ble_mesh_init(){
    esp_err_t ret = ESP_OK;

    ESP_LOGI("MESH_INIT","Init Mesh Network");

    ret = esp_ble_mesh_register_prov_callback(provisioning_callback);
    if(ret != ESP_OK){
        ESP_LOGE("MESH_INIT","Error registering provisioning callback");
        return ret;
    }
    ret = esp_ble_mesh_register_custom_model_callback(custom_ibeacon_client_callback);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error registering custom model callback");
        return ret;
    }
    ret = esp_ble_mesh_register_config_client_callback(config_client_callback);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error registering config callback");
        return ret;
    }
    ret = esp_ble_mesh_init(&provision,&composition);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error during mesh init");
        return ret;
    }

    ret = esp_ble_mesh_client_model_init(&custom_models[0]);
    if(ret != ESP_OK){
        ESP_LOGE("MESH_INIT","Error during model client init");
        return ret;
    }

    uint8_t match[2] = {0xdd, 0xdd};
    prov_key.net_idx = ESP_BLE_MESH_KEY_PRIMARY;
    prov_key.app_idx = APP_KEY_IDX;
    memset(prov_key.app_key,APP_KEY_OCTET,sizeof(prov_key.app_key));

    ret = esp_ble_mesh_provisioner_set_dev_uuid_match(match, sizeof(match),0x0,false);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error during set dev uuid match");
        return ret;
    }
    ret = esp_ble_mesh_provisioner_prov_enable(ESP_BLE_MESH_PROV_ADV);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error during enable provisioning");
        return ret;
    }
    ret = esp_ble_mesh_provisioner_add_local_app_key(prov_key.app_key,prov_key.net_idx,prov_key.app_idx);
    if(ret != ESP_OK) {
        ESP_LOGE("MESH_INIT", "Error during add local app key");
        return ret;
    }

    ESP_LOGI("MESH_INIT","Mesh Init Complete");

    return ret;
}

static void provisioning_callback(esp_ble_mesh_prov_cb_event_t event, esp_ble_mesh_prov_cb_param_t *param){
    struct ble_mesh_provisioner_prov_comp_param complete = param->provisioner_prov_complete;
    struct ble_mesh_provisioner_recv_unprov_adv_pkt_param unprov = param->provisioner_recv_unprov_adv_pkt;
    switch(event) {
        case ESP_BLE_MESH_PROVISIONER_PROV_COMPLETE_EVT:
            prov_complete(complete.node_idx,
                          complete.device_uuid,
                          complete.unicast_addr,
                          complete.element_num,
                          complete.netkey_idx);
            break;
        case ESP_BLE_MESH_PROVISIONER_RECV_UNPROV_ADV_PKT_EVT:
            recv_unprov_adv_pkt(unprov.dev_uuid,unprov.addr,unprov.addr_type,unprov.oob_info,unprov.adv_type,unprov.bearer);
            break;
        case ESP_BLE_MESH_PROVISIONER_ADD_LOCAL_APP_KEY_COMP_EVT:
            prov_key.app_idx = param->provisioner_add_app_key_comp.app_idx;
            esp_ble_mesh_provisioner_bind_app_key_to_local_model(PROV_OWN_ADDR,prov_key.app_idx,ESP_BLE_MESH_IBEACON_MODEL_ID_CLIENT,CID_ESP);
            break;
        default:
            break;
    }
}

static void config_client_callback(esp_ble_mesh_cfg_client_cb_event_t event, esp_ble_mesh_cfg_client_cb_param_t *param) {
    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_node_info_t *node = NULL;
    uint32_t opcode;
    uint16_t addr;

    opcode = param->params->opcode;
    addr = param->params->ctx.addr;
    node = ble_mesh_get_node_info(addr);

    switch (event) {
        case ESP_BLE_MESH_CFG_CLIENT_GET_STATE_EVT:
            switch (opcode) {
                case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET: {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {0};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                    set_state.app_key_add.net_idx = prov_key.net_idx;
                    set_state.app_key_add.app_idx = prov_key.app_idx;
                    memcpy(set_state.app_key_add.app_key, prov_key.app_key, 16);
                    esp_ble_mesh_config_client_set_state(&common, &set_state);
                }
                    break;
                default:
                    break;
            }
            break;
        case ESP_BLE_MESH_CFG_CLIENT_SET_STATE_EVT: {
            switch (opcode) {
                case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD: {
                    esp_ble_mesh_cfg_client_set_state_t set_state = {0};
                    ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                    set_state.model_app_bind.element_addr = node->unicast;
                    set_state.model_app_bind.model_app_idx = prov_key.app_idx;
                    set_state.model_app_bind.model_id = ESP_BLE_MESH_IBEACON_MODEL_ID_SERVER;
                    set_state.model_app_bind.company_id = CID_ESP;
                    esp_ble_mesh_config_client_set_state(&common, &set_state);
                    break;
                }
                case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND: {
                    esp_ble_mesh_generic_client_get_state_t get_state = {0};
                    ble_mesh_set_msg_common(&common, node, ibeacon_model_client.model,
                                            ESP_BLE_MESH_IBEACON_MODEL_OP_GET);
                    esp_ble_mesh_generic_client_get_state(&common, &get_state);
                    break;
                }
                default:
                    break;
            }
            break;
            case ESP_BLE_MESH_CFG_CLIENT_TIMEOUT_EVT:
                switch (opcode) {
                    case ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET: {
                        esp_ble_mesh_cfg_client_get_state_t get_state = {0};
                        ble_mesh_set_msg_common(&common, node, config_client.model,
                                                ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
                        get_state.comp_data_get.page = COMP_DATA_PAGE_0;
                        esp_ble_mesh_config_client_get_state(&common, &get_state);
                    }
                        break;
                    case ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD: {
                        esp_ble_mesh_cfg_client_set_state_t set_state = {0};
                        ble_mesh_set_msg_common(&common, node, config_client.model, ESP_BLE_MESH_MODEL_OP_APP_KEY_ADD);
                        set_state.app_key_add.net_idx = prov_key.net_idx;
                        set_state.app_key_add.app_idx = prov_key.app_idx;
                        memcpy(set_state.app_key_add.app_key, prov_key.app_key, 16);
                        esp_ble_mesh_config_client_set_state(&common, &set_state);
                    }
                        break;
                    case ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND: {
                        esp_ble_mesh_cfg_client_set_state_t set_state = {0};
                        ble_mesh_set_msg_common(&common, node, config_client.model,
                                                ESP_BLE_MESH_MODEL_OP_MODEL_APP_BIND);
                        set_state.model_app_bind.element_addr = node->unicast;
                        set_state.model_app_bind.model_app_idx = prov_key.app_idx;
                        set_state.model_app_bind.model_id = ESP_BLE_MESH_IBEACON_MODEL_ID_SERVER;
                        set_state.model_app_bind.company_id = CID_ESP;
                        esp_ble_mesh_config_client_set_state(&common, &set_state);
                    }
                        break;
                    default:
                        break;
                }
            break;
            default:
                break;
        }
    }
}

static esp_err_t  store_node_info(const uint8_t uuid[16],uint16_t unicast, uint8_t elem_num){
    for (int i = 0; i< ARRAY_SIZE(nodes); i++){
        if(!memcmp(nodes[i].uuid, uuid,16)){
            nodes[i].unicast = unicast;
            nodes[i].elem_num = elem_num;
            //ESP_LOGI("STORE","unicast %x %x",nodes[i].unicast,unicast);
            return ESP_OK;
        }
    }
    for (int i = 0; i< ARRAY_SIZE(nodes); i++){
        if(nodes[i].unicast == ESP_BLE_MESH_ADDR_UNASSIGNED){
            memcpy(nodes[i].uuid, uuid, 16);
            nodes[i].unicast = unicast;
            nodes[i].elem_num = elem_num;
            //ESP_LOGI("STORE","unicast %x %x",nodes[i].unicast,unicast);
            return ESP_OK;
        }
    }
    ESP_LOGE("STORE_NODE","Cannot store node with uuid:" );
    ESP_LOG_BUFFER_HEX_LEVEL("STORE_NODE",uuid,16,ESP_LOG_ERROR);
    return ESP_FAIL;
}

static esp_ble_mesh_node_info_t *ble_mesh_get_node_info(uint16_t unicast)
{
    int i;

    if (!ESP_BLE_MESH_ADDR_IS_UNICAST(unicast)) {
        ESP_LOGE("NODE_INFO", "Address 0x%x is not a valid unicast address",unicast);
        return NULL;
    }

    for (i = 0; i < ARRAY_SIZE(nodes); i++) {
        if (nodes[i].unicast <= unicast &&
            nodes[i].unicast + nodes[i].elem_num > unicast) {
            return &nodes[i];
        }
    }

    return NULL;
}

static esp_err_t ble_mesh_set_msg_common(esp_ble_mesh_client_common_param_t *common,
                                                 esp_ble_mesh_node_info_t *node,
                                                 esp_ble_mesh_model_t *model, uint32_t opcode)
{
    if (!common || !node || !model) {
        ESP_LOGW("MSG_COMM","Invalid Arguments");
        return ESP_ERR_INVALID_ARG;
    }

    common->opcode = opcode;
    common->model = model;
    common->ctx.net_idx = prov_key.net_idx;
    common->ctx.app_idx = prov_key.app_idx;
    common->ctx.addr = node->unicast;
    common->ctx.send_ttl = MSG_SEND_TTL;
    common->ctx.send_rel = MSG_SEND_REL;
    common->msg_timeout = MSG_TIMEOUT;
    common->msg_role = MSG_ROLE;

    return ESP_OK;
}

static esp_err_t prov_complete(int node_idx, const esp_ble_mesh_octet16_t uuid, uint16_t unicast, uint8_t elem_num, uint16_t net_idx){

    esp_ble_mesh_client_common_param_t common = {0};
    esp_ble_mesh_cfg_client_get_state_t  get_state = {0};
    esp_ble_mesh_node_info_t *node = NULL;
    char name[11] = {0};
    esp_err_t err = ESP_OK;

    ESP_LOGI("PROV COMPLETE","Provisioned device with uuid 0x%s and address %d",bt_hex(uuid,16),unicast);

    sprintf(name, "%s%d","NODE-", node_idx);
    esp_ble_mesh_provisioner_set_node_name(node_idx,name);
    store_node_info(uuid,unicast,elem_num);

    node = ble_mesh_get_node_info(unicast);
    ble_mesh_set_msg_common(&common,node,config_client.model, ESP_BLE_MESH_MODEL_OP_COMPOSITION_DATA_GET);
    err = esp_ble_mesh_config_client_get_state(&common,&get_state);
    if (err == ESP_OK)
        ESP_LOGI("PROV","Provision Completed");
    else
        ESP_LOGE("PROV","Provision error");
    return err;
}

static void recv_unprov_adv_pkt(uint8_t dev_uuid[16], uint8_t addr[BD_ADDR_LEN], esp_ble_mesh_addr_type_t addrType, uint16_t oob_info, uint8_t adv_type, esp_ble_mesh_prov_bearer_t bearer){
    esp_ble_mesh_unprov_dev_add_t add_dev = {0};
    memcpy(add_dev.addr,addr,BD_ADDR_LEN);
    add_dev.addr_type = (uint8_t) addrType;
    memcpy(add_dev.uuid,dev_uuid,16);
    add_dev.oob_info = oob_info;
    add_dev.bearer = (uint8_t) bearer;
    esp_ble_mesh_provisioner_add_unprov_dev(&add_dev,(ADD_DEV_RM_AFTER_PROV_FLAG | ADD_DEV_START_PROV_NOW_FLAG | ADD_DEV_FLUSHABLE_DEV_FLAG));
}


void ble_mesh_get_dev_uuid(uint8_t *dev_uuid){
    memcpy(dev_uuid +2, esp_bt_dev_get_address(),BD_ADDR_LEN);
    ESP_LOG_BUFFER_HEX("DEVUUID",dev_uuid,16);
}

static void custom_ibeacon_client_callback(esp_ble_mesh_model_cb_event_t event, esp_ble_mesh_model_cb_param_t *param){
    switch (event) {
        case ESP_BLE_MESH_MODEL_OPERATION_EVT:
            switch(param->model_operation.opcode){
                case ESP_BLE_MESH_IBEACON_MODEL_OP_STATUS:;
                model_ibeacon_data_t ibeacon_response = *(model_ibeacon_data_t *)param->client_recv_publish_msg.msg;
                uint8_t *uuid = get_uuid_from_addr(param->client_recv_publish_msg.ctx->addr);
                update_distance(uuid,ibeacon_response.distance);
                break;
            }
            break;
        default:
            break;
    }
}

uint8_t * get_uuid_from_addr(uint16_t addr){
    for(int i = 0; i < ARRAY_SIZE(nodes); i++){
        if(nodes[i].unicast == addr)
            return nodes[i].uuid;
    }
    return NULL;
}

esp_err_t ble_mesh_ibeacon_model_client_message_get(){
    esp_ble_mesh_msg_ctx_t ctx = {0};
    uint32_t opcode;
    esp_err_t err = ESP_OK;
    opcode = ESP_BLE_MESH_IBEACON_MODEL_OP_GET;
    for (int i = 0; i < ARRAY_SIZE(nodes); i++) {
        if (nodes[i].unicast == ESP_BLE_MESH_ADDR_UNASSIGNED)
            return ESP_OK;
        ctx.net_idx = prov_key.net_idx;
        ctx.app_idx = prov_key.app_idx;
        ctx.addr = nodes[i].unicast;
        ctx.send_ttl = 7;
        ctx.send_rel = false;
        err = esp_ble_mesh_client_model_send_msg(ibeacon_model_client.model, &ctx, opcode, 0, NULL, 0, true, ROLE_PROVISIONER);
        if (err != ESP_OK)
            ESP_LOGE("SEND_GET", "Sending error\n");
    }
    return err;
}