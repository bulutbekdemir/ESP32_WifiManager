/*!
*	@file main.c
*
*	@date 2024
* @author Bulut Bekdemir
* 
* @copyright BSD 3-Clause License
* @version 0.1.0+2
*/
#include <stdio.h>
#include <inttypes.h>

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "wifiManager.h"

void app_main(void)
{
  ///> Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if(ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	///> Get the chip information
	esp_chip_info_t chip_info;
	esp_chip_info(&chip_info);	
	ESP_LOGI("MAIN", "ESP32 Chip Rev: %d", chip_info.revision);
	ESP_LOGI("MAIN", "ESP32 Chip Cores: %d", chip_info.cores);
	ESP_LOGI("MAIN", "ESP32 Chip Features: %ld", chip_info.features);
	ESP_LOGI("MAIN", "ESP32 Chip Revision: %d", chip_info.revision);
	ESP_LOGI("MAIN", "ESP32 Chip Cores: %d", chip_info.cores);

	///> Initialize the wifi application
	wifiManager_init();

}