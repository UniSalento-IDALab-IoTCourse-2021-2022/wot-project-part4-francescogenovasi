#include "ota.h"

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 digest length */

#define TAG "OTA"

/*an ota data write buffer ready to write to the flash*/
static char ota_write_data[BUFFSIZE + 1] = {0};

void http_cleanup(esp_http_client_handle_t client) {
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
}

void print_sha256(const uint8_t *image_hash) {
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
}

void ota_task() {

    esp_err_t err;
    /* update handle : set by esp_ota_begin(), must be freed via esp_ota_end() */
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    //const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    esp_http_client_config_t config = {
            .url = OTA_UPDATE_URL,
            .timeout_ms = OTA_TIMEOUT_MS,
            .keep_alive_enable = true,
    };
    while (1) {

        vTaskDelay(pdMS_TO_TICKS(5000));

        esp_http_client_handle_t client = esp_http_client_init(&config);
        if (client == NULL) {
            continue;
        }
        err = esp_http_client_open(client, 0);
        if (err != ESP_OK) {
            esp_http_client_cleanup(client);
            continue;
        }
        esp_http_client_fetch_headers(client);

        update_partition = esp_ota_get_next_update_partition(NULL);
        assert(update_partition != NULL);

        int binary_file_length = 0;
        /*deal with all receive packet*/
        bool image_header_was_checked = false;
        while(1){
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0) {
            http_cleanup(client);
            break;
        } else if (data_read > 0) {
            if (image_header_was_checked == false) {
                esp_app_desc_t new_app_info;
                if (data_read >
                    sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t)) {
                    // check current version with downloading
                    memcpy(&new_app_info,
                           &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)],
                           sizeof(esp_app_desc_t));

                    esp_app_desc_t running_app_info;
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK) {
                    }

                    const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK) {
                    }

                    // check current version with last invalid partition
                    if (last_invalid_app != NULL) {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) ==
                            0) {
                            http_cleanup(client);
                            break;
                        }
                    }
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0) {
                        http_cleanup(client);
                        break;
                    }

                    image_header_was_checked = true;

                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    if (err != ESP_OK) {
                        http_cleanup(client);
                        esp_ota_abort(update_handle);
                        break;
                    }
                } else {
                    http_cleanup(client);
                    esp_ota_abort(update_handle);
                    break;
                }
            }
            err = esp_ota_write(update_handle, (const void *) ota_write_data, data_read);
            if (err != ESP_OK) {
                http_cleanup(client);
                esp_ota_abort(update_handle);
                break;
            }
            binary_file_length += data_read;
        } else if (data_read == 0) {
            /*
             * As esp_http_client_read never returns negative error code, we rely on
             * `errno` to check for underlying transport connectivity closure if any
             */
            if (errno == ECONNRESET || errno == ENOTCONN) {
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) {
                break;
            }
        }
        }
        if (esp_http_client_is_complete_data_received(client) != true) {
            esp_ota_abort(update_handle);
            continue;
        }

        err = esp_ota_end(update_handle);
        if (err != ESP_OK) {
            http_cleanup(client);
            continue;
        }

        err = esp_ota_set_boot_partition(update_partition);
        if (err != ESP_OK) {
            http_cleanup(client);
            continue;
        }
        esp_restart();
        //return;
    }
}