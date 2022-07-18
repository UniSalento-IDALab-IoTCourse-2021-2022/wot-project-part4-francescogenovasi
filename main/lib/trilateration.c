#include "triangulation.h"
#include <math.h>

 void update_distance(uint8_t *uuid, float d){
        for(int i = 0; i<3; i++){
            if(memcmp(uuid,fixed_nodes[i].uuid,16) == 0) {
                fixed_nodes[i].distance = d;
                //ESP_LOG_BUFFER_HEX("TRILA",fixed_nodes[i].uuid,16);
                //ESP_LOG_BUFFER_HEX("TRILA",uuid,16);
                ESP_LOGI("TRILA","Dispositivo: %d Distance: %f",i,d);
            }
        }
}

// TODO modificare calcolo stima x e y del beacon

void estimate_position(){
    float a[3], b[3], c[3];
    float x;
    float y;
    a[0] = 2 * (fixed_nodes[1].x - fixed_nodes[0].x)
    a[1] = 2 * (fixed_nodes[2].x - fixed_nodes[0].x)
    a[2] = 2 * (fixed_nodes[2].x - fixed_nodes[1].x)

    b[0] = 2 * (fixed_nodes[1].y - fixed_nodes[0].y)
    b[1] = 2 * (fixed_nodes[2].y - fixed_nodes[0].y)
    b[2] = 2 * (fixed_nodes[2].y - fixed_nodes[1].y)

    // C1 = d1^2 - d2^2 + x2^2 -x1^2 + y2^2 - y1^2
    c[0] = pow(fixed_nodes[0].distance, 2) - pow(fixed_nodes[1].distance, 2) + pow(fixed_nodes[1].x, 2) - pow(fixed_nodes[0].x, 2) + pow(fixed_nodes[1].y, 2) - pow(fixed_nodes[0].y, 2)
    // C2 = d1^2 - d3^2 + x3^2 -x1^2 + y3^2 - y1^2
    c[1] = pow(fixed_nodes[0].distance, 2) - pow(fixed_nodes[2].distance) + pow(fixed_nodes[2].x, 2) - pow(fixed_nodes[0].x, 2) + pow(fixed_nodes[2].y, 2) - pow(fixed_nodes[0].y, 2)
    // C3 = d2^2 - d3^2 + x3^2 -x2^2 + y3^2 - y2^2
    c[2] = pow(fixed_nodes[1].distance, 2) - pow(fixed_nodes[3].distance, 2) + pow(fixed_nodes[2].x, 2) - pow(fixed_nodes[1].x, 2) + pow(fixed_nodes[2].y, 2) - pow(fixed_nodes[1].y, 2)

    // https://ieeexplore.ieee.org/document/9165464 section II

    float matr_A[2][2], matr_B[2], matr_A_inv[2][2], matr_cof_A[2][2], matr_cof_A_t[2][2];

    matr_A[0][0] = pow(a[0], 2) + pow(a[1], 2) + pow(a[2], 2)
    matr_A[0][1] = a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
    matr_A[1][0] = a[0]*b[0] + a[1]*b[1] + a[2]*b[2]
    matr_A[1][1] = pow(b[0], 2) + pow(b[1], 2) + pow(b[2], 2)

    matr_B[0] = a[0]*c[0] + a[1]*c[1] + a[2]*c[2]
    matr_B[1] = b[0]*c[0] + b[1]*c[1] + b[2]*c[2]

    //controllo matrice invertibile -> determinante diverso da 0
    float det = (matr_A[0][0] * matr_A[1][1]) - (matr_A[0][1] * matr_A[1][0])

    if (det == 0){
        // TODO errore, matrice non invertibile!
    }

    // matrice cofattori
    matr_cof_A[0][0] = 1 * matr_A[1][1];
    matr_cof_A[0][1] = (-1) * matr_A[1][0];
    matr_cof_A[1][0] = (-1) * matr_A[0][1];
    matr_cof_A[1][1] = 1 * matr_A[0][0];

    // matrice cofattori trasposta
    matr_cof_A_t[0][0] = matr_cof_A[0][0];
    matr_cof_A_t[0][1] = matr_cof_A[1][0];
    matr_cof_A_t[1][0] = matr_cof_A[0][1];
    matr_cof_A_t[1][1] = matr_cof_A[1][1];

    // matrice inversa
    matr_A_inv[0][0] = (1/det) * matr_cof_A[0][0];
    matr_A_inv[0][1] = (1/det) * matr_cof_A[0][1];
    matr_A_inv[1][0] = (1/det) * matr_cof_A[1][0];
    matr_A_inv[1][1] = (1/det) * matr_cof_A[1][1];


    x = (matr_A_inv[0][0] * matr_B[0]) + (matr_A_inv[0][1] * matr_B[1])
    y = (matr_A_inv[1][0] * matr_B[0]) + (matr_A_inv[1][1] * matr_B[1])

    ESP_LOGI("TRILAT", "Estimated Beacon position: X: %f, Y: %f", x, y);
    mqtt_publish(x, y, fixed_nodes[0].distance, fixed_nodes[1].distance, fixed_nodes[2].distance);
}