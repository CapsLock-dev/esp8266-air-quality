#include "my_wifi_handler.h"
#include "my_mqtt.h"
#include "my_i2c.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "esp_sleep.h"

static const char* MAIN_TAG = "MAIN";

#define SLEEP_MINUTES 	1
#define SLEEP_TIME 		(SLEEP_MINUTES * 60 * 1000000)
#define HUM_PREC 		CONFIG_HUM_PRECISION
#define TEMP_PREC 		CONFIG_TEMP_PRECISION

int power_of_ten(uint8_t exponent) {
    switch(exponent) {
        case 0:  return 1;
        case 1:  return 10;
        case 2:  return 100;
        case 3:  return 1000;
        case 4:  return 10000;
        case 5:  return 100000;
        case 6:  return 1000000;
        case 7:  return 10000000;
        case 8:  return 100000000;
        case 9:  return 1000000000;
        default: return 0;
    }
}

void app_main() {
    ESP_ERROR_CHECK(nvs_flash_init());
	ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

	init_wifi();
	start_i2c();

    double temp=0, hum=0;
	uint8_t aqi=0;
    uint16_t tvoc=0, eco2=0;
	esp_err_t aht_ret = aht20_read(&temp, &hum);     
    esp_err_t ens_ret = ens160_read(&aqi, &tvoc, &eco2);

	if (aht_ret == ESP_OK && ens_ret == ESP_OK) {
		start_mqtt();
		esp_mqtt_client_handle_t mqtt_client = get_mqtt_client();
		mqtt_send_data(mqtt_client, (int)(temp*power_of_ten(atoi(TEMP_PREC))), (int)(hum*power_of_ten(atoi(HUM_PREC))), aqi, tvoc, eco2);
	} else {	
		if (aht_ret != ESP_OK) ESP_LOGE(MAIN_TAG, "Got error while reading from aht20: %d", aht_ret);
		if (ens_ret != ESP_OK) ESP_LOGE(MAIN_TAG, "Got error while reading from ens160: %d", ens_ret);
	}
	ESP_LOGI(MAIN_TAG, "Entering sleep for %d minutes", SLEEP_MINUTES);
	stop_i2c();
	stop_wifi();
	stop_mqtt();
	esp_sleep_enable_timer_wakeup(SLEEP_TIME);
    esp_light_sleep_start();
	esp_restart();
}
