#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_err.h"

#include "driver/i2c.h"

void start_i2c();
void stop_i2c();

esp_err_t aht20_read(double* temp, double* hum);

esp_err_t ens160_read(uint8_t *aqi, uint16_t *tvoc, uint16_t *eco2);
