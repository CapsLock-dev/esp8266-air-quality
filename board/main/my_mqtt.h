#ifndef MY_MQTT_H
#define MY_MQTT_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_event.h" 

#include "mqtt_client.h"

void start_mqtt();
void mqtt_send_data(esp_mqtt_client_handle_t client, int temp, int hum, uint8_t aqi, uint16_t tvoc, uint16_t eco2);
esp_mqtt_client_handle_t get_mqtt_client();
void stop_mqtt();

#endif
