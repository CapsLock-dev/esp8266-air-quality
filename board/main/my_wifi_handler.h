#ifndef MY_WIFI_HANDLER_H
#define MY_WIFI_HANDLER_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_event.h" 
#include "esp_wifi.h"

void init_wifi();
void stop_wifi();

#endif
