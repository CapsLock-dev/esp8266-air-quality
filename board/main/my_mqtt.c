#include "my_mqtt.h"

static const char *TAG = "MQTT_EXAMPLE";
static esp_mqtt_client_handle_t s_mqtt_client = NULL;
static EventGroupHandle_t s_mqtt_event_group = NULL;

#define MQTT_CONNECTED_BIT 	BIT0
#define MQTT_SENT_BIT 		BIT1
#define MQTT_BROKER_URI 	CONFIG_MQTT_BROKER_URL

esp_mqtt_client_handle_t get_mqtt_client() {
    return s_mqtt_client;
}

void mqtt_send_data(esp_mqtt_client_handle_t client, int temp, int hum, uint8_t aqi, uint16_t tvoc, uint16_t eco2) {
	if (!client) return;
	char payload[150];
    snprintf(payload, sizeof(payload),
             "{\"temp\":%d,\"hum\":%d,\"aqi\":%d,\"tvoc\":%d,\"eco2\":%d}",
             temp, hum, aqi, tvoc, eco2); 
    int msg_id = esp_mqtt_client_publish(client, "/sensors/environment", payload, 0, 1, 0);
    xEventGroupWaitBits(s_mqtt_event_group, MQTT_SENT_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

static esp_err_t mqtt_event_handler_cb(esp_mqtt_event_handle_t event) {
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch (event->event_id) {
        case MQTT_EVENT_CONNECTED:
			s_mqtt_client = client;
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
			xEventGroupSetBits(s_mqtt_event_group, MQTT_CONNECTED_BIT);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
			s_mqtt_client = NULL;
            break;
 		case MQTT_EVENT_PUBLISHED:
            ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED");
			xEventGroupSetBits(s_mqtt_event_group, MQTT_SENT_BIT);
        default:
            break;
    }
    return ESP_OK;
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    mqtt_event_handler_cb(event_data);
}

void start_mqtt() {
	esp_mqtt_client_config_t mqtt_cfg = {
		.uri = MQTT_BROKER_URI,
	};
    s_mqtt_event_group = xEventGroupCreate();
	esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
    xEventGroupWaitBits(s_mqtt_event_group, MQTT_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}

void stop_mqtt() {
	if (s_mqtt_client) {
		esp_mqtt_client_disconnect(s_mqtt_client);
		esp_mqtt_client_stop(s_mqtt_client);
		esp_mqtt_client_destroy(s_mqtt_client);
		s_mqtt_client = NULL;
	}
}
