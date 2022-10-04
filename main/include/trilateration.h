#ifndef MESH_NETWORK_PROVISIONER_TRILATERATION_H
#define MESH_NETWORK_PROVISIONER_TRILATERATION_H
#include <stdlib.h>
#include <string.h>
#include "mqtt.h"
#include <esp_log.h>

typedef struct {
    float x;
    float y;
    uint8_t uuid[16];
    float distance;
}device_node_t;

// TODO modificare posizioni dei server node

static device_node_t fixed_nodes[3] = {{.x=2.6f,.y=0.0f,.uuid={0xdd, 0xdd, 0xc8, 0xc9, 0xa3, 0xca, 0x2e, 0xc6, 0x00,0x00,0x00,0x00, 0x00,0x0,0x00,0x00}, .distance=0},
                                       {.x=0.0f,.y=0.0f,.uuid={0xdd, 0xdd, 0x78, 0xe3, 0x6d, 0x0a, 0x5a, 0xfa, 0x00,0x00,0x00,0x00, 0x00,0x0,0x00,0x00}, .distance=0},
                                       {.x=0.0f,.y=2.08f,.uuid={0xdd, 0xdd, 0xc8, 0xc9, 0xa3, 0xc6, 0x89, 0xd6, 0x00,0x00,0x00,0x00, 0x00,0x0,0x00,0x00}, .distance=0}};

void update_distance(uint8_t *uuid,float d);
void estimate_position();

#endif //MESH_NETWORK_PROVISIONER_TRILATERATION_H
