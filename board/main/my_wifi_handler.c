#include "my_wifi_handler.h"

#define WIFI_SSID       CONFIG_WIFI_SSID
#define WIFI_PASS 		CONFIG_WIFI_PASSWORD
#define WIFI_CONNECTED_BIT BIT0

static EventGroupHandle_t s_connect_event_group;

static const char* TAG = "WIFI";

static void on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    esp_wifi_connect();
}

static void on_wifi_connect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
	ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
	ESP_LOGI(TAG, "got ip:%s", ip4addr_ntoa(&event->ip_info.ip));
	xEventGroupSetBits(s_connect_event_group, WIFI_CONNECTED_BIT);

    vEventGroupDelete(s_connect_event_group);
}

void init_wifi() {	
    s_connect_event_group = xEventGroupCreate();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_connect, NULL));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
			.threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
	ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_NONE));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_connect());
	
    xEventGroupWaitBits(s_connect_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);

    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &on_wifi_connect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &on_wifi_disconnect));
    vEventGroupDelete(s_connect_event_group);
}

void stop_wifi() {
	esp_wifi_stop();
}
