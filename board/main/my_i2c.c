#include "my_i2c.h"

#define I2C_MASTER_SCL_IO 5
#define I2C_MASTER_SDA_IO 4
#define I2C_POWER_GPIO 14  

#define ACK_CHECK_EN                        0x1              /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS                       0x0              /*!< I2C master will not check ack from slave */

#define ACK_VAL                             0x0              /*!< I2C ack value */
#define NACK_VAL                            0x1              /*!< I2C nack value */
#define LAST_NACK_VAL                       0x2     

#define AHT20_ADDR			0x38
#define AHT20_STATUS_BUSY 	0x80

#define ENS160_ADDR				0x53
#define ENS160_REG_OPMODE		0x10
#define ENS160_REG_COMMAND		0x12
#define ENS160_REG_DATA_AQI		0x21
#define ENS160_REG_DATA_STATUS		0x20

#define ENS160_COMMAND_NOP		0x00
#define ENS160_COMMAND_CLRGPR	0xCC

#define ENS160_OPMODE_RESET		0xF0
#define ENS160_OPMODE_IDLE		0x01
#define ENS160_OPMODE_STD		0x02

static const char *TAG = "I2C";

static void i2c_init() {
    i2c_config_t conf = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = I2C_MASTER_SDA_IO,
    	.sda_pullup_en = 1,
    	.scl_io_num = I2C_MASTER_SCL_IO,
    	.scl_pullup_en = 1,
    	.clk_stretch_tick = 300, 
	};
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, conf.mode));
    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &conf));
}

static esp_err_t write_to_device(uint8_t* data, size_t data_len, uint8_t address, int8_t register_addr) {
	i2c_cmd_handle_t cmd_handle = i2c_cmd_link_create();
	i2c_master_start(cmd_handle);
	i2c_master_write_byte(cmd_handle, address<< 1 | I2C_MASTER_WRITE, ACK_CHECK_EN);
	if (register_addr >= 0) i2c_master_write_byte(cmd_handle, register_addr, ACK_CHECK_EN);
	i2c_master_write(cmd_handle, data, data_len, ACK_CHECK_EN);
	i2c_master_stop(cmd_handle);
	esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd_handle, 1000 / portTICK_RATE_MS);
	i2c_cmd_link_delete(cmd_handle);
	
	return ret;
}

static esp_err_t read_from_device(uint8_t *data, size_t data_len, uint8_t address, int8_t register_addr) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_WRITE, ACK_CHECK_EN);
    if (register_addr >= 0) {
        i2c_master_write_byte(cmd, register_addr, ACK_CHECK_EN);
    }
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (address << 1) | I2C_MASTER_READ, ACK_CHECK_EN);
	i2c_master_read(cmd, data, data_len, LAST_NACK_VAL);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin(I2C_NUM_0, cmd, 1000 / portTICK_PERIOD_MS);
    i2c_cmd_link_delete(cmd);
    return ret;
}

static void ens160_init() {
	uint8_t reset_cmd = ENS160_OPMODE_RESET;
    uint8_t idle_cmd = ENS160_OPMODE_IDLE;
    uint8_t nop_cmd = ENS160_COMMAND_NOP;
    uint8_t clrgpr_cmd = ENS160_COMMAND_CLRGPR;
	ESP_ERROR_CHECK(write_to_device(&reset_cmd, 1, ENS160_ADDR, ENS160_REG_OPMODE)); // reset
	vTaskDelay(100 / portTICK_RATE_MS);
	ESP_ERROR_CHECK(write_to_device(&idle_cmd, 1, ENS160_ADDR, ENS160_REG_OPMODE)); // set idle mode
	vTaskDelay(20 / portTICK_PERIOD_MS);
	ESP_ERROR_CHECK(write_to_device(&nop_cmd, 1, ENS160_ADDR, ENS160_REG_COMMAND));
	ESP_ERROR_CHECK(write_to_device(&clrgpr_cmd, 1, ENS160_ADDR, ENS160_REG_COMMAND));
 	vTaskDelay(20 / portTICK_PERIOD_MS);
	uint8_t std_cmd = ENS160_OPMODE_STD;  // Standard measurement mode
	ESP_ERROR_CHECK(write_to_device(&std_cmd, 1, ENS160_ADDR, ENS160_REG_OPMODE));
    vTaskDelay(20 / portTICK_PERIOD_MS);
}


esp_err_t ens160_read(uint8_t *aqi, uint16_t *tvoc, uint16_t *eco2) {
	uint8_t status;
    
    // Read status register
    if (read_from_device(&status, 1, ENS160_ADDR, ENS160_REG_DATA_STATUS) != ESP_OK) {
        return ESP_FAIL;
    }
    
    // Check if new data is available (bit 0)
    if (!(status & 0x01)) {
        return ESP_ERR_INVALID_STATE;
    }
	uint8_t data[5];
    if (read_from_device(data, sizeof(data), ENS160_ADDR, ENS160_REG_DATA_AQI) != ESP_OK) {
        return ESP_FAIL;
    }
    
    *aqi = data[0];
    *tvoc = (data[2] << 8) | data[1];
    *eco2 = (data[4] << 8) | data[3];
    
    return ESP_OK;
}

static void aht20_init() {
	uint8_t init_cmd[3] = {0xBE, 0x08, 0x00};
	ESP_ERROR_CHECK(write_to_device(init_cmd, sizeof(init_cmd), AHT20_ADDR, -1));
}

esp_err_t aht20_read(double* temp, double* hum) {
	uint8_t start_measurement_cmd[3] = {0xAC, 0x33, 0x00};
	esp_err_t ec = write_to_device(start_measurement_cmd, sizeof(start_measurement_cmd), AHT20_ADDR, -1);
	if (ec != ESP_OK) return ec;
	vTaskDelay(80 / portTICK_RATE_MS);
	uint8_t data[6] = {0};

	do {
		ec = read_from_device(data, sizeof(data), AHT20_ADDR, -1);
		if (ec != ESP_OK) return ec;
		if (data[0] & AHT20_STATUS_BUSY) vTaskDelay(80 / portTICK_RATE_MS);
	} while(data[0] & AHT20_STATUS_BUSY);

	uint32_t h = data[1]; 
	h <<= 8; 
  	h |= data[2]; 
  	h <<= 4; 
  	h |= data[3] >> 4; 
  	*hum = ((double)h * 100) / 0x100000;

	uint32_t tdata = data[3] & 0x0F; 
  	tdata <<= 8;
  	tdata |= data[4];
  	tdata <<= 8;
  	tdata |= data[5];
  	*temp = ((double)tdata * 200 / 0x100000) - 50;

    return ESP_OK;
}

void start_i2c() {
	gpio_set_direction(I2C_POWER_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(I2C_POWER_GPIO, 1);
    vTaskDelay(pdMS_TO_TICKS(250));  

	i2c_init();	
	vTaskDelay(100 / portTICK_RATE_MS); 

	aht20_init();
	ens160_init();
	vTaskDelay(1000 / portTICK_RATE_MS);
}

void stop_i2c() {
	i2c_driver_delete(I2C_NUM_0);
    gpio_set_level(I2C_POWER_GPIO, 0);
}
